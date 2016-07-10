/***************************************************************
**
**  Xinwei Telecom Technology co., ltd. ShenZhen R&D center
**
**  Core Network Department  platform team
**
**  filename:       prefix_match_tree.h
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
#ifndef PREFIX_MATCH_TREE_H_
#define PREFIX_MATCH_TREE_H_
#include <string>
#include <vector>

#define XSTATIC static
#define XCONST const
typedef unsigned char XU8;
typedef char XS8;
typedef unsigned int XU32;
typedef int XS32;

//前缀树符号表抽象类，
class CAbstractSymbolMap
{
public:
    virtual XU8 GetIndex(XS8 symbol) = 0;
    virtual XU8 GetSymbolNum() = 0; // 保证符号数量不超过256
};

// 一般不使用，常用接口封装到类CFacadePrefixMactchTree
class CPrefixMatchTree : CAbstractSymbolMap
{
public:
    class CNode;
    typedef void (*FuncNodeForEach)(CNode *node, void* param);
    typedef ::std::vector<CNode *> VectorNode;
    typedef VectorNode::iterator VectorNodeIter;
    class CNode
    {
        friend class CPrefixMatchTree;
    public:
        void* data() {
            return data_;
        }
        XU32 deepth() {
            return deepth_;
        }
        XU32 pass_times() {
            return pass_times_;
        }
        ::std::string GetPassSymbols();
    private:
        CNode(CPrefixMatchTree *tree_info);
        ~CNode();
        CNode* AddChild(XS8 symbol);
        CNode* FindChild(XS8 symbol);
        XSTATIC void RemoveNode(CNode* data_node, XU32 is_rollback = 0);
        void set_data(XCONST void* data);
        void ClearChildNode();
        XS8 symbol_;
        XU8 index_;
        void* data_;
        CNode* father_;
        XU32 pass_times_; // 匹配的数据节点个数,包含当前数据节点
        XU32 deepth_; // 树高度
        VectorNode nodes_;
        CPrefixMatchTree *tree_info_;
    };

    CPrefixMatchTree(CAbstractSymbolMap* node_map, XU32 fid, XU32 data_len);
    virtual ~CPrefixMatchTree();
    XU8 GetIndex(XS8 symbol);
    XU8 GetSymbolNum();
    XS32 InsertData(XCONST XS8* symbols, XCONST void* data);
    CNode* FindNode(XCONST XS8* symbols, XU32 is_query = 0);
    XCONST void* FindData(XCONST XS8* symbols);
    void RemoveData(XCONST XS8* symbols);
    void DataNodeForEach(XCONST XS8* symbols, FuncNodeForEach func,  void* param = 0);
    void FindAllDataNode(XCONST XS8* symbols, VectorNode& node_container, CNode* node=0);
    void MatchClear(XCONST XS8* symbols);
    XU32 fid() {
        return fid_;
    }
    XU32 data_len() {
        return data_len_;
    }
    XU32 node_counter() {
        return node_counter_;
    };
    XU32 prefix_num() {
        return root_->pass_times();
    };
private:
    CAbstractSymbolMap* symbol_map_;
    XU32 fid_;
    XU32 data_len_;
    XU32 node_counter_; // 记录节点总数，用于配平资源参考
    CNode *root_;
};

// 数字字符串符号表实现
class CNumberSymbolMap : public CAbstractSymbolMap
{
public:
    CNumberSymbolMap();
    ~CNumberSymbolMap();
    XU8 GetIndex(XS8 symbol);
    XU8 GetSymbolNum();
};

class CAsciiSymbolMap : public CAbstractSymbolMap
{
public:
    CAsciiSymbolMap();
    ~CAsciiSymbolMap();
    XU8 GetIndex(XS8 symbol);
    XU8 GetSymbolNum();
};


// 提供前缀匹配树通用接口
// 目前支持字符串输入
// 线程安全
template<typename DATA>
class CFacadePrefixMactchTree
{
public:
    CFacadePrefixMactchTree(CAbstractSymbolMap* symbol_map, XU32 fid)
        :tree_(symbol_map, fid, sizeof(DATA))
    {
    }
    ~CFacadePrefixMactchTree() {
    }
    XS32 InsertData(XCONST XS8* symbols, XCONST DATA* data)
    {
        XS32 ret = tree_.InsertData(symbols, data);
        return ret;
    }
    XCONST DATA* FindData(XCONST XS8* symbols)
    {
        XCONST DATA* data = reinterpret_cast<XCONST DATA*>(tree_.FindData(symbols)) ;
        return data;
    }
    void RemoveData(XCONST XS8* symbols)
    {
        tree_.RemoveData(symbols);
    }
    void DataNodeForEach(XCONST XS8* symbols, CPrefixMatchTree::FuncNodeForEach func,  void* param = 0)
    {
        tree_.DataNodeForEach(symbols, func, param);
    }

    CPrefixMatchTree tree_;
};
#endif // PREFIX_MATCH_TREE_H_