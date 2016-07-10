/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename:       prefix_match_tree.cpp
**  description:    前缀匹配树实现
**  author:         luozhongjie
**  data:           2014.10.21
**
***************************************************************
**                          history
**
***************************************************************
**   author          date              modification
**   luozhongjie     2014.10.21        create
**************************************************************/
#include "prefix_match_tree.h"


/*lint -e578*/ // Warning 578 Declaration of symbol 'data' hides symbol
CPrefixMatchTree::CNode::CNode(CPrefixMatchTree *tree_info)
    :symbol_(0), index_(0), data_(0), father_(0), pass_times_(0),
     deepth_(0), nodes_(tree_info->GetSymbolNum(), 0), tree_info_(tree_info)
{
    /*lint -e1060*/  // private member 'CPrefixMatchTree::node_counter_' is not accessible to non-member non-friend functions
    tree_info->node_counter_ ++;
}
CPrefixMatchTree::CNode::~CNode()
{

    try
    {
        tree_info_->node_counter_ --;
        if(data_)
        { 
            free(data_);
        }
    }
    catch(...) {}
    /*lint +e1060*/
}
CPrefixMatchTree::CNode* CPrefixMatchTree::CNode::AddChild(XS8 symbol)
{
    XU8 index = tree_info_->GetIndex(symbol);
    if(index >= nodes_.size())
    {
        RemoveNode(this, 1); // 添加数据节点失败时回滚
        return 0;
    }
    CNode* new_node = nodes_[index];
    if(!new_node)
    {
        new_node = new CNode(tree_info_);
        nodes_[index] = new_node;
        new_node->father_ = this;
        new_node->deepth_ = deepth_ + 1;
        new_node->symbol_ = symbol;
        new_node->index_ = index;
    }
    return new_node;
}
CPrefixMatchTree::CNode* CPrefixMatchTree::CNode::FindChild(XS8 symbol)
{
    XU8 index = tree_info_->GetIndex(symbol);
    if(index >= nodes_.size()) return 0;
    return nodes_[index];
}
void CPrefixMatchTree::CNode::RemoveNode(CNode* data_node, XU32 is_rollback)
{
    if(!data_node) return ;
    if(!is_rollback) 
    {
        if(!data_node->data_) // 只能删除有数据的节点
            return ;
        free(data_node->data_);
        data_node->data_ = 0;
    }
    CNode *cur_node = data_node;
    CNode* father;
    while (cur_node)
    {
        if(!is_rollback)  --cur_node->pass_times_; // 如果删除节点有数据 所有途径上的节点计数-1
        father = cur_node->father_;
        if(cur_node->pass_times_ == 0 && father)
        {
            father->nodes_[cur_node->index_] = 0;
            delete cur_node;
        }
        cur_node = father;
    }
}
void CPrefixMatchTree::CNode::set_data(XCONST void* data)
{
    if(!data) return ;
    if(!data_)
    {
        data_ = (void*)malloc(tree_info_->data_len());
        if(!data_)
        {
            RemoveNode(this, 1);
            return;
        }
        CNode* cur_node = this; // 新增数据节点时，所有经过的节点计数加1
        while(cur_node)
        {
            cur_node->pass_times_ ++;
            cur_node = cur_node->father_;
        }
    }
    if(data_) memcpy(data_, data, tree_info_->data_len());
}
void CPrefixMatchTree::CNode::ClearChildNode()
{
    CNode *current_node;
    VectorNode::iterator iter = nodes_.begin();
    unsigned i = pass_times();
    if(data_) // 当前节点为数据节点
    {
        --i;
        RemoveNode(this); // 会造成父节点释放
    }
    while(i) // 遍历所有子节点
    {
        current_node = *(iter++);
        if(current_node)
        {
            i -= current_node->pass_times();
            current_node->ClearChildNode();
        }
    }
}
::std::string CPrefixMatchTree::CNode::GetPassSymbols()
{
    XU32 i = deepth_;
    std::string symbols(i, '0');
    CNode* current_node = this;
    while(i--)
    {
        symbols[i] = current_node->symbol_;
        current_node = current_node->father_;
    }
    return symbols;
}
CPrefixMatchTree::CPrefixMatchTree(CAbstractSymbolMap* node_map, XU32 fid, XU32 data_len)
    :symbol_map_(node_map),fid_(fid),data_len_(data_len),node_counter_(0)
{
    root_ = new CNode(this);
}
CPrefixMatchTree::~CPrefixMatchTree()
{
    try
    {
        root_->ClearChildNode();
        delete root_;
        delete symbol_map_;
    }
    catch(...) {}
}
XU8 CPrefixMatchTree::GetIndex(XS8 symbol)
{
    return symbol_map_->GetIndex(symbol);
}
XU8 CPrefixMatchTree::GetSymbolNum()
{
    return symbol_map_->GetSymbolNum();
}
XS32 CPrefixMatchTree::InsertData(XCONST XS8* symbols, XCONST void* data)
{
    CNode* cur_node = root_;
    try
    {
        if(!symbols || !data) return -1;
        CNode* child;
        while(*symbols)
        {
            child = cur_node->AddChild(*symbols);
            if(!child) return -1;
            cur_node = child;
            ++symbols;
        }
        cur_node->set_data(data);
    }
    catch(...)
    {
        CNode::RemoveNode(cur_node, 1); // 添加数据节点失败时回滚,内存不够
        return -1;
    }
    return 0;
}
// is_query为0时，匹配节点，为1时查询节点
CPrefixMatchTree::CNode* CPrefixMatchTree::FindNode(XCONST XS8* symbols, XU32 is_query)
{
    if(!symbols) return root_;
    CNode* cur_node = root_;
    CNode* matched_node = 0;
    while(*symbols)
    {
        cur_node = cur_node->FindChild(*symbols);
        if(!cur_node) break;
        if(cur_node->data_) matched_node=cur_node;
        ++symbols;
    }
    return is_query ? cur_node : matched_node;
}
XCONST void* CPrefixMatchTree::FindData(XCONST XS8* symbols)
{
    CNode* node = FindNode(symbols);
    return node ? node->data() : 0 ;
}
void CPrefixMatchTree::RemoveData(XCONST XS8* symbols)
{
    CNode* node = FindNode(symbols, 0);
    if(node && node->data())
    {
        CNode::RemoveNode(node);
    }
}
void CPrefixMatchTree::DataNodeForEach(XCONST XS8* symbols, FuncNodeForEach node_func, void* param)
{
    if(!node_func) return ;
    VectorNode node_container;
    FindAllDataNode(symbols, node_container);
    CNode* node;
    for(VectorNodeIter iter=node_container.begin(); iter !=node_container.end(); ++iter)
    {
        node = *iter;
        node_func(node, param);
    }
}
void CPrefixMatchTree::FindAllDataNode(XCONST XS8* symbols, VectorNode& node_container, CNode* node)
{
    CNode* current = root_;
    if(node) current = node;
    else if(symbols)
    {
        current = FindNode(symbols, 1); // 查询节点
        if(!current )
        {
            current = FindNode(symbols); // 匹配节点
            if(!current) return ;
            else
            {
                node_container.push_back(current);
                return ;
            }
        }
    }
    unsigned i = current->pass_times();
    if(current->data_) // 当前节点为数据节点
    {
        node_container.push_back(current);
        --i;
    }
    VectorNodeIter iter = current->nodes_.begin();
    while(i) // 遍历所有子节点
    {
        current = *(iter++);
        if(current)
        {
#ifdef WIN32
            if(i < current->pass_times())
                fprintf(stderr, "exception\r\n");
#endif
            i -= current->pass_times();
            FindAllDataNode(0, node_container, current);
        }
    }
}
void CPrefixMatchTree::MatchClear(XCONST XS8* symbols)
{
    CNode* node = FindNode(symbols, 1);
    node->ClearChildNode();
}
CNumberSymbolMap::CNumberSymbolMap() {}
CNumberSymbolMap::~CNumberSymbolMap() {}
XU8 CNumberSymbolMap::GetIndex(XS8 symbol)
{
    XU8 index = symbol-'0';
    return index <= GetSymbolNum() ? index : GetSymbolNum()+1;
}
XU8 CNumberSymbolMap::GetSymbolNum()
{
    return 10;
}

CAsciiSymbolMap::CAsciiSymbolMap() {}
CAsciiSymbolMap::~CAsciiSymbolMap() {}
XU8 CAsciiSymbolMap::GetIndex(XS8 symbol)
{
    XU8 index = symbol;
    return index <= GetSymbolNum() ? index : GetSymbolNum()+1;
}
XU8 CAsciiSymbolMap::GetSymbolNum()
{
    return 127;
}