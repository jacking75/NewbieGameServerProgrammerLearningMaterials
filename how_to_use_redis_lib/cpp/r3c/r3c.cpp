// Writed by yijian (eyjian@gmail.com)
// R3C is a C++ client for redis based on hiredis (https://github.com/redis/hiredis)
#include "r3c.h"
#include "utils.h"
#include <assert.h>

#ifdef _MSC_VER
#pragma warning(disable:4267)
#endif

#define R3C_ASSERT assert
#define THROW_REDIS_EXCEPTION(errinfo) \
    throw CRedisException(errinfo, __FILE__, __LINE__)
#define THROW_REDIS_EXCEPTION_WITH_NODE(errinfo, node_ip, node_port) \
    throw CRedisException(errinfo, __FILE__, __LINE__, node_ip, node_port)
#define THROW_REDIS_EXCEPTION_WITH_NODE_AND_COMMAND(errinfo, node_ip, node_port, command, key) \
    throw CRedisException(errinfo, __FILE__, __LINE__, node_ip, node_port, command, key)

namespace r3c {

int NUM_RETRIES = 15; // The default number of retries is 15 (CLUSTERDOWN cost more than 6s)
int CONNECT_TIMEOUT_MILLISECONDS = 2000; // Connection timeout in milliseconds
int READWRITE_TIMEOUT_MILLISECONDS = 2000; // Receive and send timeout in milliseconds

#if R3C_TEST // for test
    static LOG_WRITE g_error_log = r3c_log_write;
    static LOG_WRITE g_info_log = r3c_log_write;
    static LOG_WRITE g_debug_log = r3c_log_write;
#else
    static LOG_WRITE g_error_log = null_log_write;
    static LOG_WRITE g_info_log = null_log_write;
    static LOG_WRITE g_debug_log = null_log_write;
#endif // R3C_TEST

void set_error_log_write(LOG_WRITE info_log)
{
    g_error_log = info_log;
    if (NULL == g_error_log)
        g_error_log = null_log_write;
}

void set_info_log_write(LOG_WRITE info_log)
{
    g_info_log = info_log;
    if (NULL == g_info_log)
        g_info_log = null_log_write;
}

void set_debug_log_write(LOG_WRITE debug_log)
{
    g_debug_log = debug_log;
    if (NULL == g_debug_log)
        g_debug_log = null_log_write;
}

static int get_retry_sleep_milliseconds(int loop_counter)
{
    static const int sleep_milliseconds_table[] = { 10, 100, 200, 500, 1000 };
    if (loop_counter<0 || loop_counter>=static_cast<int>(sizeof(sleep_milliseconds_table)/sleep_milliseconds_table[0]-1))
        return 1000;
    else
        return sleep_milliseconds_table[loop_counter];
}

enum
{
    CLUSTER_SLOTS = 16384 // number of slots, defined in cluster.h
};

std::string zaddflag2str(ZADDFLAG zaddflag)
{
    std::string zaddflag_str;

    switch (zaddflag)
    {
    case Z_XX:
        zaddflag_str = "XX";
        break;
    case Z_NX:
        zaddflag_str = "NX";
        break;
    case Z_CH:
        zaddflag_str = "CH";
        break;
    default:
        break;
    }

    return zaddflag_str;
}

size_t NodeHasher::operator ()(const Node& node) const
{
    const unsigned char* s = reinterpret_cast<const unsigned char*>(node.first.c_str());
    const uint64_t l = static_cast<uint64_t>(node.first.size());
    uint64_t crc = 0;
    return static_cast<size_t>(crc64(crc, s, l));
}

std::string& node2string(const Node& node, std::string* str)
{
    *str = node.first + std::string(":") + int2string(node.second);
    return *str;
}

std::string node2string(const Node& node)
{
    std::string nodestr;
    return node2string(node, &nodestr);
}

std::string NodeInfo::str() const
{
    return format_string("nodeinfo://%s/%s:%d/%s", id.c_str(), node.first.c_str(), node.second, flags.c_str());
}

bool NodeInfo::is_master() const
{
    const std::string::size_type pos = flags.find("master");
    return (pos != std::string::npos);
}

bool NodeInfo::is_replica() const
{
    const std::string::size_type pos = flags.find("slave");
    return (pos != std::string::npos);
}

bool NodeInfo::is_fail() const
{
    const std::string::size_type pos = flags.find("fail");
    return (pos != std::string::npos);
}

////////////////////////////////////////////////////////////////////////////////
// CCommandArgs

CommandArgs::CommandArgs()
    : _argc(0), _argv(NULL), _argvlen(NULL)
{
}

CommandArgs::~CommandArgs()
{
    delete []_argvlen;
    for (int i=0; i<_argc; ++i)
        delete []_argv[i];
    delete []_argv;
}

void CommandArgs::set_key(const std::string& key)
{
    _key = key;
}

void CommandArgs::set_command(const std::string& command)
{
    _command = command;
}

void CommandArgs::add_arg(const std::string& arg)
{
    _args.push_back(arg);
}

void CommandArgs::add_arg(int32_t arg)
{
    _args.push_back(int2string(arg));
}

void CommandArgs::add_arg(uint32_t arg)
{
    _args.push_back(int2string(arg));
}

void CommandArgs::add_arg(int64_t arg)
{
    _args.push_back(int2string(arg));
}

void CommandArgs::add_args(const std::vector<std::string>& args)
{
    for (std::vector<std::string>::size_type i=0; i<args.size(); ++i)
    {
        const std::string& arg = args[i];
        add_arg(arg);
    }
}

void CommandArgs::add_args(const std::vector<std::pair<std::string, std::string> >& values)
{
    for (std::vector<std::string>::size_type i=0; i<values.size(); ++i)
    {
        const std::string& field = values[i].first;
        const std::string& value = values[i].second;
        add_arg(field);
        add_arg(value);
    }
}

void CommandArgs::add_args(const std::map<std::string, std::string>& map)
{
    for (std::map<std::string, std::string>::const_iterator iter=map.begin(); iter!=map.end(); ++iter)
    {
        add_arg(iter->first);
        add_arg(iter->second);
    }
}

void CommandArgs::add_args(const std::map<std::string, int64_t>& map, bool reverse)
{
    for (std::map<std::string, int64_t>::const_iterator iter=map.begin(); iter!=map.end(); ++iter)
    {
        if (!reverse)
        {
            add_arg(iter->first);
            add_arg(iter->second);
        }
        else
        {
            add_arg(iter->second);
            add_arg(iter->first);
        }
    }
}

void CommandArgs::add_args(const std::vector<FVPair>& fvpairs)
{
    for (std::vector<FVPair>::size_type i=0; i<fvpairs.size(); ++i)
    {
        const FVPair& fvpair = fvpairs[i];
        add_arg(fvpair.field);
        add_arg(fvpair.value);
    }
}

void CommandArgs::final()
{
    _argc = static_cast<int>(_args.size());
    _argv = new char*[_argc];
    _argvlen = new size_t[_argc];

    for (int i=0; i<_argc; ++i)
    {
        _argvlen[i] = _args[i].size();
        _argv[i] = new char[_argvlen[i]+1];
        memcpy(_argv[i], _args[i].c_str(), _argvlen[i]); // Support binary key&value.
        _argv[i][_argvlen[i]] = '\0';
    }
}

int CommandArgs::get_argc() const
{
    return _argc;
}

const char** CommandArgs::get_argv() const
{
    return (const char**)_argv;
}

const size_t* CommandArgs::get_argvlen() const
{
    return _argvlen;
}

const std::string& CommandArgs::get_command() const
{
    return _command;
}

const std::string& CommandArgs::get_key() const
{
    return _key;
}

////////////////////////////////////////////////////////////////////////////////
// CRedisNode
// CRedisMasterNode
// CRedisReplicaNode

class CRedisNode
{
public:
    CRedisNode(const NodeId& nodeid, const Node& node, redisContext* redis_context)
        : _nodeid(nodeid),
          _node(node),
          _redis_context(redis_context),
          _conn_errors(0)
    {
    }

    ~CRedisNode()
    {
        close();
    }

    const NodeId& get_nodeid() const
    {
        return _nodeid;
    }

    const Node& get_node() const
    {
        return _node;
    }

    redisContext* get_redis_context() const
    {
        return _redis_context;
    }

    void set_redis_context(redisContext* redis_context)
    {
        _redis_context = redis_context;
        if (NULL == _redis_context)
        {
            ++_conn_errors;
            //(*g_debug_log)("%s\n", str().c_str());
        }
    }

    void close()
    {
        if (_redis_context != NULL)
        {
            redisFree(_redis_context);
            _redis_context = NULL;
        }
    }

    std::string str() const
    {
        return format_string("node://(connerrors:%u)%s:%d", _conn_errors, _node.first.c_str(), _node.second);
    }

    unsigned int get_conn_errors() const
    {
        return _conn_errors;
    }

    void inc_conn_errors()
    {
        ++_conn_errors;
    }

    void reset_conn_errors()
    {
        _conn_errors = 0;
    }

    void set_conn_errors(unsigned int conn_errors)
    {
        _conn_errors = conn_errors;
    }

    bool need_refresh_master() const
    {
        return ((_conn_errors>3 && 0==_conn_errors%3) || (_conn_errors>2018));
    }

protected:
    NodeId _nodeid;
    Node _node;
    redisContext* _redis_context;
    unsigned int _conn_errors; // 连续连接失败数
};

class CRedisMasterNode;

class CRedisReplicaNode: public CRedisNode
{
public:
    CRedisReplicaNode(const NodeId& node_id, const Node& node, redisContext* redis_context)
        : CRedisNode(node_id, node, redis_context),
          _redis_master_node(NULL)
    {
    }

private:
    CRedisMasterNode* _redis_master_node;
};

class CRedisMasterNode: public CRedisNode
{
public:
    CRedisMasterNode(const NodeId& node_id, const Node& node, redisContext* redis_context)
        : CRedisNode(node_id, node, redis_context),
          _index(0)
    {
    }

    ~CRedisMasterNode()
    {
        clear();
    }

    void clear()
    {
        for (RedisReplicaNodeTable::iterator iter=_redis_replica_nodes.begin(); iter!=_redis_replica_nodes.end(); ++iter)
        {
            CRedisReplicaNode* replica_node = iter->second;
            delete replica_node;
        }
        _redis_replica_nodes.clear();
    }

    void add_replica_node(CRedisReplicaNode* redis_replica_node)
    {
        const Node& node = redis_replica_node->get_node();
        const std::pair<RedisReplicaNodeTable::iterator, bool> ret = _redis_replica_nodes.insert(std::make_pair(node, redis_replica_node));
        if (!ret.second)
        {
            CRedisReplicaNode* old_replica_node = ret.first->second;
            delete old_replica_node;
            ret.first->second = redis_replica_node;
        }
    }

    CRedisNode* choose_node(ReadPolicy read_policy)
    {
        const unsigned int num_redis_replica_nodes = static_cast<unsigned int>(_redis_replica_nodes.size());
        CRedisNode* redis_node = NULL;

        if (0 == num_redis_replica_nodes)
        {
            redis_node = this;
        }
        else
        {
            unsigned int K = _index++ % (num_redis_replica_nodes+1); // Included master

            if (RP_READ_REPLICA==read_policy && K==num_redis_replica_nodes)
            {
                redis_node = this;
            }
            else
            {
                RedisReplicaNodeTable::iterator iter = _redis_replica_nodes.begin();

                for (unsigned int i=0; i<K; ++i)
                {
                    if (++iter == _redis_replica_nodes.end())
                        iter = _redis_replica_nodes.begin();
                }
                if (iter != _redis_replica_nodes.end())
                {
                    redis_node = iter->second;
                }
                if (NULL == redis_node)
                {
                    redis_node = this;
                }
            }
        }

        return redis_node;
    }

private:
    typedef std::unordered_map<Node, CRedisReplicaNode*, NodeHasher> RedisReplicaNodeTable;

    RedisReplicaNodeTable _redis_replica_nodes;
    unsigned int _index;
};

////////////////////////////////////////////////////////////////////////////////
// RedisReplyHelper

RedisReplyHelper::RedisReplyHelper()
    : _redis_reply(NULL)
{
}

RedisReplyHelper::RedisReplyHelper(const redisReply* redis_reply)
    : _redis_reply(redis_reply)
{
}

RedisReplyHelper::RedisReplyHelper(const RedisReplyHelper& other)
{
    _redis_reply = other.detach();
}

RedisReplyHelper::~RedisReplyHelper()
{
    free();
}

RedisReplyHelper::operator bool() const
{
    return _redis_reply != NULL;
}

void RedisReplyHelper::free()
{
    if (_redis_reply != NULL)
    {
        freeReplyObject((void*)_redis_reply);
        _redis_reply = NULL;
    }
}

const redisReply* RedisReplyHelper::get() const
{
    return _redis_reply;
}

const redisReply* RedisReplyHelper::detach() const
{
    const redisReply* redis_reply = _redis_reply;
    _redis_reply = NULL;
    return redis_reply;
}

RedisReplyHelper& RedisReplyHelper::operator =(const redisReply* redis_reply)
{
    free();
    _redis_reply = redis_reply;
    return *this;
}

RedisReplyHelper& RedisReplyHelper::operator =(const RedisReplyHelper& other)
{
    free();
    _redis_reply = other.detach();
    return *this;
}

const redisReply* RedisReplyHelper::operator ->() const
{
    return _redis_reply;
}

std::ostream& RedisReplyHelper::operator <<(std::ostream& os)
{
    os << *_redis_reply;
    return os;
}

////////////////////////////////////////////////////////////////////////////////
// ErrorInfo

ErrorInfo::ErrorInfo()
    : errcode(0)
{
}

ErrorInfo::ErrorInfo(const std::string& raw_errmsg_, const std::string& errmsg_, const std::string& errtype_, int errcode_)
    : raw_errmsg(raw_errmsg_), errmsg(errmsg_), errtype(errtype_), errcode(errcode_)
{
}

void ErrorInfo::clear()
{
    errcode = 0;
    errtype.clear();
    errmsg.clear();
    raw_errmsg.clear();
}

////////////////////////////////////////////////////////////////////////////////
// CRedisException

CRedisException::CRedisException(
        const struct ErrorInfo& errinfo,
        const char* file, int line,
        const std::string& node_ip, uint16_t node_port,
        const std::string& command, const std::string& key) throw ()
    :  _errinfo(errinfo),
       _line(line),
       _node_ip(node_ip), _node_port(node_port),
       _command(command), _key(key)
{
    const char* slash_position = strrchr(file, '/');
    const std::string* file_cp = &_file;
    std::string* file_p = const_cast<std::string*>(file_cp);

    if (NULL == slash_position)
    {
        *file_p = file;
    }
    else
    {
        *file_p = std::string(slash_position + 1);
    }
}

const char* CRedisException::what() const throw()
{
    return _errinfo.errmsg.c_str();
}

std::string CRedisException::str() const throw ()
{
    const std::string& errmsg = format_string("redis_exception://%s:%d/CMD:%s/%s/(%d)%s@%s:%d",
            _node_ip.c_str(), _node_port,
            _command.c_str(),
            _errinfo.errtype.c_str(), _errinfo.errcode, _errinfo.errmsg.c_str(),
            _file.c_str(), _line);

#if __cplusplus < 201103L
    return errmsg;
#else
    return std::move(errmsg);
#endif // __cplusplus < 201103L
}

bool is_ask_error(const std::string& errtype)
{
    return (errtype.size() == sizeof("ASK")-1) && (errtype == "ASK");
}

bool is_clusterdown_error(const std::string& errtype)
{
    // CLUSTERDOWN The cluster is down
    // 如果在从未成为主之前，将主和从同时干掉，则即使从正常了，从也无法成为主，
    // 这个时候必须将原来的主恢复，否则该部分slots就不能再服务
    return (errtype.size() == sizeof("CLUSTERDOWN")-1) && (errtype == "CLUSTERDOWN");
}

bool is_moved_error(const std::string& errtype)
{
    return (errtype.size() == sizeof("MOVED")-1) && (errtype == "MOVED");
}

bool is_noauth_error(const std::string& errtype)
{
    // NOAUTH Authentication required.
    return (errtype.size() == sizeof("NOAUTH")-1) && (errtype == "NOAUTH");
}

bool is_noscript_error(const std::string& errtype)
{
    // NOSCRIPT No matching script. Please use EVAL.
    return (errtype.size() == sizeof("NOSCRIPT")-1) && (errtype == "NOSCRIPT");
}

bool is_wrongtype_error(const std::string& errtype)
{
    return (errtype.size() == sizeof("WRONGTYPE")-1) && (errtype == "WRONGTYPE");
}

bool is_busygroup_error(const std::string& errtype)
{
    return (errtype.size() == sizeof("BUSYGROUP")-1) && (errtype == "BUSYGROUP");
}

bool is_nogroup_error(const std::string& errtype)
{
    return (errtype.size() == sizeof("NOGROUP")-1) && (errtype == "NOGROUP");
}

bool is_crossslot_error(const std::string& errtype)
{
    return (errtype.size() == sizeof("CROSSSLOT")-1) && (errtype == "CROSSSLOT");
}

////////////////////////////////////////////////////////////////////////////////
// CRedisClient

CRedisClient::CRedisClient(
        const std::string& raw_nodes_string,
        int connect_timeout_milliseconds,
        int readwrite_timeout_milliseconds,
        const std::string& password,
        ReadPolicy read_policy
        )
            : _command_monitor(NULL),
              _raw_nodes_string(raw_nodes_string),
              _connect_timeout_milliseconds(connect_timeout_milliseconds),
              _readwrite_timeout_milliseconds(readwrite_timeout_milliseconds),
              _password(password),
              _read_policy(read_policy)
{
    init();
}

CRedisClient::CRedisClient(
        const std::string& raw_nodes_string,
        const std::string& password,
        int connect_timeout_milliseconds,
        int readwrite_timeout_milliseconds,
        ReadPolicy read_policy)
            : _command_monitor(NULL),
              _raw_nodes_string(raw_nodes_string),
              _connect_timeout_milliseconds(connect_timeout_milliseconds),
              _readwrite_timeout_milliseconds(readwrite_timeout_milliseconds),
              _password(password),
              _read_policy(read_policy)
{
    init();
}

CRedisClient::CRedisClient(
        const std::string& raw_nodes_string,
        ReadPolicy read_policy,
        const std::string& password,
        int connect_timeout_milliseconds,
        int readwrite_timeout_milliseconds)
            : _command_monitor(NULL),
              _raw_nodes_string(raw_nodes_string),
              _connect_timeout_milliseconds(connect_timeout_milliseconds),
              _readwrite_timeout_milliseconds(readwrite_timeout_milliseconds),
              _password(password),
              _read_policy(read_policy)
{
    init();
}

CRedisClient::~CRedisClient()
{
    fini();
}

const std::string& CRedisClient::get_raw_nodes_string() const
{
    return _raw_nodes_string;
}

const std::string& CRedisClient::get_nodes_string() const
{
    return _nodes_string;
}

std::string CRedisClient::str() const
{
    if (cluster_mode())
        return std::string("rediscluster://") + _raw_nodes_string;
    else
        return std::string("redisstandalone://") + _raw_nodes_string;
}

bool CRedisClient::cluster_mode() const
{
    return _nodes.size() > 1;
}

const char* CRedisClient::get_mode_str() const
{
    return cluster_mode()? "CLUSTER": "STANDALONE";
}

void CRedisClient::enable_debug_log()
{
    _enable_debug_log = true;
}

void CRedisClient::disable_debug_log()
{
    _enable_debug_log = false;
}

void CRedisClient::enable_info_log()
{
    _enable_info_log = true;
}

void CRedisClient::disable_info_log()
{
    _enable_info_log = false;
}

void CRedisClient::enable_error_log()
{
    _enable_error_log = true;
}

void CRedisClient::disable_error_log()
{
    _enable_error_log = false;
}

int CRedisClient::list_nodes(std::vector<struct NodeInfo>* nodes_info)
{
    struct ErrorInfo errinfo;

    for (RedisMasterNodeTable::iterator iter=_redis_master_nodes.begin(); iter!=_redis_master_nodes.end(); ++iter)
    {
        const Node& node = iter->first;
        CRedisNode* redis_node = iter->second;
        redisContext* redis_context = redis_node->get_redis_context();

        if (redis_context != NULL)
        {
            if (list_cluster_nodes(nodes_info, &errinfo, redis_context, node))
                break;
        }
    }
    if (nodes_info->empty())
    {
        THROW_REDIS_EXCEPTION(errinfo);
    }

    return static_cast<int>(nodes_info->size());
}

void CRedisClient::flushall()
{
    const int num_retries = NUM_RETRIES;
    const std::string key;
    CommandArgs cmd_args;
    cmd_args.set_command("FLUSHALL");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.final();

    // Simple string reply (REDIS_REPLY_STATUS)
    redis_command(true, num_retries, key, cmd_args, NULL);
}

void CRedisClient::multi(const std::string& key, Node* which)
{
    if (cluster_mode())
    {
        struct ErrorInfo errinfo;
        errinfo.errcode = ERROR_NOT_SUPPORT;
        errinfo.errmsg = "MULTI not supported in cluster mode";
        THROW_REDIS_EXCEPTION(errinfo);
    }
    else
    {
        const int num_retries = 0;
        CommandArgs cmd_args;
        cmd_args.set_key(key);
        cmd_args.set_command("MULTI");
        cmd_args.add_arg(cmd_args.get_command());
        cmd_args.final();

        // Simple string reply (REDIS_REPLY_STATUS):
        // always OK.
        redis_command(false, num_retries, key, cmd_args, which);
    }
}

const RedisReplyHelper CRedisClient::exec(const std::string& key, Node* which)
{
    if (cluster_mode())
    {
        struct ErrorInfo errinfo;
        errinfo.errcode = ERROR_NOT_SUPPORT;
        errinfo.errmsg = "EXEC not supported in cluster mode";
        THROW_REDIS_EXCEPTION(errinfo);
    }
    else
    {
        const int num_retries = 0;
        CommandArgs cmd_args;
        cmd_args.set_key(key);
        cmd_args.set_command("EXEC");
        cmd_args.add_arg(cmd_args.get_command());
        cmd_args.final();

        // Array reply:
        // each element being the reply to each of the commands in the atomic transaction.
        return redis_command(false, num_retries, key, cmd_args, which);
    }
}

//
// KEY/VALUE
//

// Time complexity: O(1)
// EXPIRE key seconds
bool CRedisClient::expire(
        const std::string& key,
        uint32_t seconds,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("EXPIRE");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(seconds);
    cmd_args.final();

    // Integer reply, specifically:
    // 1 if the timeout was set.
    // 0 if key does not exist.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return 1 == redis_reply->integer;
    return true;
}

// Time complexity: O(1)
// EXISTS key [key ...]
bool CRedisClient::exists(const std::string& key, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("EXISTS");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Integer reply, specifically:
    // 1 if the key exists.
    // 0 if the key does not exist.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return 1 == redis_reply->integer;
    return true;
}

// Time complexity:
// O(N) where N is the number of keys that will be removed.
// When a key to remove holds a value other than a string,
// the individual complexity for this key is O(M) where M is the number of elements in the list, set, sorted set or hash.
// Removing a single key that holds a string value is O(1).
bool CRedisClient::del(const std::string& key, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("DEL");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Integer reply:
    // The number of keys that were removed.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return 1 == redis_reply->integer;
    return true;
}

// GET key
// Time complexity: O(1)
bool CRedisClient::get(
        const std::string& key,
        std::string* value,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("GET");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Bulk string reply:
    // the value of key, or nil when key does not exist.
    RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
        return false;
    if (REDIS_REPLY_STRING == redis_reply->type)
        return get_value(redis_reply.get(), value);
    return true;
}

// SET key value [EX seconds] [PX milliseconds] [NX|XX]
// Time complexity: O(1)
void CRedisClient::set(
        const std::string& key,
        const std::string& value,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SET");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Simple string reply (REDIS_REPLY_STATUS):
    // OK if SET was executed correctly.

    // Null reply: a Null Bulk Reply is returned if the SET operation was not performed
    // because the user specified the NX or XX option but the condition was not met.
    //
    // OK: redis_reply->str
    redis_command(false, num_retries, key, cmd_args, which);
}

// Time complexity: O(1)
// SETNX key value
bool CRedisClient::setnx(
        const std::string& key,
        const std::string& value,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SETNX");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Integer reply, specifically:
    // 1 if the key exists.
    // 0 if the key does not exist.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return 1 == redis_reply->integer;
    return true;
}

// Time complexity: O(1)
// SETEX key seconds value
void CRedisClient::setex(
        const std::string& key,
        const std::string& value,
        uint32_t expired_seconds,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SETEX");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(expired_seconds);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Simple string reply (REDIS_REPLY_STATUS)
    //
    // OK: redis_reply->str
    redis_command(false, num_retries, key, cmd_args, which);
}

bool CRedisClient::setnxex(
        const std::string& key,
        const std::string& value,
        uint32_t expired_seconds,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SET");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(value);
    cmd_args.add_arg("EX");
    cmd_args.add_arg(expired_seconds);
    cmd_args.add_arg("NX");
    cmd_args.final();

    // Simple string reply (REDIS_REPLY_STATUS):
    // OK if SET was executed correctly.
    //
    // Null reply: a Null Bulk Reply is returned if the SET operation was not performed
    // because the user specified the NX or XX option but the condition was not met.
    //
    // OK: redis_reply->str
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    return redis_reply->type != REDIS_REPLY_NIL;
}

// Time complexity: O(N) where N is the number of keys to retrieve.
// MGET key [key ...]
int CRedisClient::mget(
        const std::vector<std::string>& keys,
        std::vector<std::string>* values,
        Node* which,
        int num_retries)
{
    values->clear();

    if (!cluster_mode())
    {
        const std::string key;
        CommandArgs cmd_args;
        cmd_args.set_command("MGET");
        cmd_args.add_arg(cmd_args.get_command());
        cmd_args.add_args(keys);
        cmd_args.final();

        // Array reply:
        // list of values at the specified keys.
        //
        // For every key that does not hold a string value or does not exist,
        // the special value nil is returned. Because of this, the operation never fails.
        const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
        if (REDIS_REPLY_ARRAY == redis_reply->type)
            return get_values(redis_reply.get(), values);
        return 0;
    }
    else
    {
        values->resize(keys.size());

        try
        {
            for (std::vector<std::string>::size_type i=0; i<keys.size(); ++i)
            {
                const std::string& key = keys[i];
                std::string& value = (*values)[i];
                get(key, &value, NULL, num_retries);
            }
        }
        catch (CRedisException&)
        {
            values->clear();
            throw;
        }

        return static_cast<int>(values->size());
    }
}

// Time complexity:
// O(N) where N is the number of keys to set.
// MSET key value [key value ...]
int CRedisClient::mset(
        const std::map<std::string, std::string>& kv_map,
        Node* which,
        int num_retries)
{
    int success = 0;

    if (kv_map.empty())
    {
        struct ErrorInfo errinfo;
        errinfo.errcode = ERROR_PARAMETER;
        errinfo.errmsg = "kv_map is empty";
        THROW_REDIS_EXCEPTION(errinfo);
    }
    if (!cluster_mode())
    {
        const std::string key;
        CommandArgs cmd_args;
        cmd_args.set_command("MSET");
        cmd_args.add_arg(cmd_args.get_command());
        cmd_args.add_args(kv_map);
        cmd_args.final();

        // Simple string reply:
        // always OK since MSET can't fail.
        redis_command(false, num_retries, key, cmd_args, which);
        success = static_cast<int>(kv_map.size());
    }
    else
    {
        for (std::map<std::string, std::string>::const_iterator iter=kv_map.begin(); iter!=kv_map.end(); ++iter)
        {
            const std::string& key = iter->first;
            const std::string& value = iter->second;
            set(key, value, which ,num_retries);
            ++success;
        }
    }

    return success;
}

// Time complexity: O(1)
// INCRBY key increment
int64_t CRedisClient::incrby(
        const std::string& key,
        int64_t increment,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("INCRBY");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(increment);
    cmd_args.final();

    // Integer reply:
    // the value of key after the increment
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return redis_reply->integer;
    return 0;
}

int64_t CRedisClient::incrby(
        const std::string& key,
        int64_t increment, int64_t expired_increment, uint32_t expired_seconds,
        Node* which,
        int num_retries)
{
    // 注意ARGV[2]和n需类型相同才可以比较，所以要么n转成字符串，要么ARGV[2]转成数字
    const std::string lua_scripts =
            "local n;n=redis.call('INCRBY',KEYS[1],ARGV[1]);"
            "if (n==tonumber(ARGV[2])) then redis.call('EXPIRE',KEYS[1],ARGV[3]) end;return n;";
    std::vector<std::string> parameters(3);
    parameters[0] = int2string(increment);
    parameters[1] = int2string(expired_increment);
    parameters[2] = int2string(expired_seconds);
    const RedisReplyHelper redis_reply = eval(key, lua_scripts, parameters, which, num_retries);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int64_t>(redis_reply->integer);
    return 0;
}

int64_t CRedisClient::incrby(
        const std::string& key,
        int64_t increment, uint32_t expired_seconds,
        Node* which,
        int num_retries)
{
    const int64_t expired_increment = increment;
    return incrby(key, increment, expired_increment, expired_seconds, which, num_retries);
}

bool CRedisClient::key_type(const std::string& key, std::string* key_type, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("TYPE");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Simple string reply (REDIS_REPLY_STATUS):
    // type of key, or none when key does not exist.
    //
    // (gdb) p *redis_reply._redis_reply
    // $1 = {type = 5, integer = 0, len = 6, str = 0x742b50 "string", elements = 0, element = 0x0}
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    return get_value(redis_reply.get(), key_type);
}

int64_t CRedisClient::ttl(const std::string& key, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("TTL");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Integer reply:
    // TTL in seconds, or a negative value in order to signal an error
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return redis_reply->integer;
    return 0;
}

// O(1) for every call. O(N) for a complete iteration,
// including enough command calls for the cursor to return back to 0. N is the number of elements inside the collection.
//
// SCAN cursor [MATCH pattern] [COUNT count]
int64_t CRedisClient::scan(int64_t cursor, std::vector<std::string>* values, Node* which, int num_retries)
{
    return scan(cursor, std::string(""), 0, values, which, num_retries);
}

// SCAN cursor [MATCH pattern] [COUNT count]
int64_t CRedisClient::scan(int64_t cursor, int count, std::vector<std::string>* values, Node* which, int num_retries)
{
    return scan(cursor, std::string(""), count, values, which, num_retries);
}

// SCAN cursor [MATCH pattern] [COUNT count]
int64_t CRedisClient::scan(
        int64_t cursor, const std::string& pattern,
        std::vector<std::string>* values,
        Node* which,
        int num_retries)
{
    return scan(cursor, pattern, 0, values, which, num_retries);
}

// SCAN cursor [MATCH pattern] [COUNT count]
int64_t CRedisClient::scan(
        int64_t cursor, const std::string& pattern, int count,
        std::vector<std::string>* values,
        Node* which,
        int num_retries)
{
    const std::string key;
    CommandArgs cmd_args;
    cmd_args.set_command("SCAN");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(cursor);
    if (!pattern.empty())
    {
        cmd_args.add_arg("MATCH");
        cmd_args.add_arg(pattern);
    }
    if (count > 0)
    {
        cmd_args.add_arg("COUNT");
        cmd_args.add_arg(count);
    }
    cmd_args.final();

    // return value is an array of two values:
    // the first value is the new cursor to use in the next call, the second value is an array of elements.
    //
    // *EMPTY*:
    // (gdb) p *redis_reply
    // $1 = {type = 2, integer = 0, len = 0, str = 0x0, elements = 2, element = 0x63e450}
    // (gdb) p *redis_reply->element[0]
    // $3 = {type = 1, integer = 0, len = 1, str = 0x63e6f0 "0", elements = 0, element = 0x0}
    // (gdb) p *redis_reply->element[1]
    // $4 = {type = 2, integer = 0, len = 0, str = 0x0, elements = 0, element = 0x0}
    //
    // *NON EMPTY*:
    //
    // redis-cli set k1 1
    // redis-cli set k2 2
    // redis-cli set k3 3
    //
    // (gdb) p *redis_reply
    // $5 = {type = 2, integer = 0, len = 0, str = 0x0, elements = 2, element = 0x63e450}
    // (gdb) p *redis_reply->element[0]
    // $6 = {type = 1, integer = 0, len = 1, str = 0x63e720 "1", elements = 0, element = 0x0}
    // (gdb) p *redis_reply->element[1]
    // $7 = {type = 2, integer = 0, len = 0, str = 0x0, elements = 3, element = 0x63e780}
    // (gdb) p *redis_reply->element[1]->element[0]
    // $8 = {type = 1, integer = 0, len = 2, str = 0x63e820 "k2", elements = 0, element = 0x0}
    // (gdb) p *redis_reply->element[1]->element[1]
    // $9 = {type = 1, integer = 0, len = 2, str = 0x63e880 "k3", elements = 0, element = 0x0}
    // (gdb) p *redis_reply->element[1]->element[2]
    // $10 = {type = 1, integer = 0, len = 2, str = 0x63e8e0 "k1", elements = 0, element = 0x0}
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
    {
        get_values(redis_reply->element[1], values);
        return static_cast<int64_t>(atoll(redis_reply->element[0]->str));
    }
    return 0;
}

//
// EVAL
//

// Time complexity: Depends on the script that is executed.
// EVAL script numkeys key [key ...] arg [arg ...]
//
// e.g.
// eval("r3c_k1", "local v=redis.call('set','r3c_k1','123');return v;");
// eval("r3c_k1", "local v=redis.call('get','r3c_k1');return v;");
// Time complexity: Depends on the script that is executed.
// EVAL script numkeys key [key ...] arg [arg ...]
//
// e.g.
// eval("r3c_k1", "local v=redis.call('set','r3c_k1','123');return v;");
// eval("r3c_k1", "local v=redis.call('get','r3c_k1');return v;");
const RedisReplyHelper CRedisClient::eval(
        const std::string& key,
        const std::string& lua_scripts,
        Node* which,
        int num_retries)
{
    const int numkeys = 1;
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("EVAL");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(lua_scripts);
    cmd_args.add_arg(numkeys);
    cmd_args.add_arg(key);
    cmd_args.final();

    return redis_command(false, num_retries, key, cmd_args, which);
}

const RedisReplyHelper CRedisClient::eval(
        const std::string& key,
        const std::string& lua_scripts,
        const std::vector<std::string>& parameters,
        Node* which,
        int num_retries)
{
    const int numkeys = 1;
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("EVAL");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(lua_scripts);
    cmd_args.add_arg(numkeys);
    cmd_args.add_arg(key);
    cmd_args.add_args(parameters);
    cmd_args.final();

    return redis_command(false, num_retries, key, cmd_args, which);
}

const RedisReplyHelper CRedisClient::evalsha(
        const std::string& key,
        const std::string& sha1,
        const std::vector<std::string>& parameters,
        Node* which, int num_retries)
{
    const int numkeys = 1;
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("EVALSHA");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(sha1);
    cmd_args.add_arg(numkeys);
    cmd_args.add_arg(key);
    cmd_args.add_args(parameters);
    cmd_args.final();

    return redis_command(false, num_retries, key, cmd_args, which);
}

const RedisReplyHelper CRedisClient::eval(
        const std::string& lua_scripts,
        const std::vector<std::string>& keys, const std::vector<std::string>& parameters,
        Node* which,
        int num_retries)
{
    if (cluster_mode() && keys_crossslots(keys))
    {
        struct ErrorInfo errinfo;
        errinfo.errcode = ERROR_NOT_SUPPORT;
        errinfo.errmsg = "EVAL not supported in cluster mode";
        THROW_REDIS_EXCEPTION(errinfo);
    }
    else
    {
        const int numkeys = static_cast<int>(keys.size());
        std::string key;
        if (cluster_mode() && !keys.empty())
            key = keys[0];

        CommandArgs cmd_args;
        if (!key.empty())
            cmd_args.set_key(key);
        cmd_args.set_command("EVAL");
        cmd_args.add_arg(cmd_args.get_command());
        cmd_args.add_arg(lua_scripts);
        cmd_args.add_arg(numkeys);
        cmd_args.add_args(keys);
        cmd_args.add_args(parameters);
        cmd_args.final();
        return redis_command(false, num_retries, key, cmd_args, which);
    }
}

const RedisReplyHelper CRedisClient::evalsha(
        const std::string& sha1,
        const std::vector<std::string>& keys,
        const std::vector<std::string>& parameters,
        Node* which,
        int num_retries)
{
    if (cluster_mode() && keys_crossslots(keys))
    {
        struct ErrorInfo errinfo;
        errinfo.errcode = ERROR_NOT_SUPPORT;
        errinfo.errmsg = "EVAL not supported in cluster mode";
        THROW_REDIS_EXCEPTION(errinfo);
    }
    else
    {
        const int numkeys = static_cast<int>(keys.size());
        std::string key;
        if (cluster_mode() && !keys.empty())
            key = keys[0];

        CommandArgs cmd_args;
        if (!key.empty())
            cmd_args.set_key(key);
        cmd_args.set_command("EVALSHA");
        cmd_args.add_arg(cmd_args.get_command());
        cmd_args.add_arg(sha1);
        cmd_args.add_arg(numkeys);
        cmd_args.add_args(keys);
        cmd_args.add_args(parameters);
        cmd_args.final();
        return redis_command(false, num_retries, key, cmd_args, which);
    }
}

//
// HASH
//

// Time complexity: O(N) where N is the number of fields to be removed.
// HDEL key field [field ...]
bool CRedisClient::hdel(const std::string& key, const std::string& field, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HDEL");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(field);
    cmd_args.final();

    // Integer reply:
    // the number of fields that were removed from the hash, not including specified but non existing fields.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
        return false;
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return 1 == redis_reply->integer;
    return true;
}

int CRedisClient::hdel(
        const std::string& key,
        const std::vector<std::string>& fields,
        Node* which,
        int num_retries)
{
    return hmdel(key, fields, which, num_retries);
}

int CRedisClient::hmdel(
        const std::string& key,
        const std::vector<std::string>& fields,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HDEL");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_args(fields);
    cmd_args.final();

    // Integer reply:
    // the number of fields that were removed from the hash, not including specified but non existing fields.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
        return 0;
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Time complexity: O(1)
// HEXISTS key field
bool CRedisClient::hexists(const std::string& key, const std::string& field, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HEXISTS");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(field);
    cmd_args.final();

    // Integer reply, specifically:
    // 1 if the hash contains field.
    // 0 if the hash does not contain field, or key does not exist.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return 1 == redis_reply->integer;
    return true;
}

// Time complexity: O(1)
// HLEN key
int CRedisClient::hlen(const std::string& key, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HLEN");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Integer reply: number of fields in the hash, or 0 when key does not exist.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Time complexity: O(1)
// HSET key field value
bool CRedisClient::hset(
        const std::string& key,
        const std::string& field,
        const std::string& value,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HSET");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(field);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Integer reply, specifically:
    // 1 if field is a new field in the hash and value was set.
    // 0 if field already exists in the hash and the value was updated.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return 1 == redis_reply->integer;
    return true;
}

bool CRedisClient::hsetex(
        const std::string& key,
        const std::string& field,
        const std::string& value,
        uint32_t expired_seconds,
        Node* which,
        int num_retries)
{
    const std::string lua_scripts =
            "local n;n=redis.call('HSET',KEYS[1],ARGV[1],ARGV[2]);"
            "if (n>0) then redis.call('EXPIRE',KEYS[1],ARGV[3]) end;return n;";
    std::vector<std::string> parameters(3);
    parameters[0] = field;
    parameters[1] = value;
    parameters[2] = int2string(expired_seconds);

    const RedisReplyHelper redis_reply = eval(key, lua_scripts, parameters, which, num_retries);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return 1 == redis_reply->integer;
    return true;
}

// HSETNX key field value
// Time complexity: O(1)
bool CRedisClient::hsetnx(
        const std::string& key,
        const std::string& field,
        const std::string& value,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HSETNX");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(field);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Integer reply, specifically:
    // 1 if field is a new field in the hash and value was set.
    // 0 if field already exists in the hash and no operation was performed.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return 1 == redis_reply->integer;
    return true;
}

bool CRedisClient::hsetnxex(
        const std::string& key,
        const std::string& field,
        const std::string& value,
        uint32_t expired_seconds,
        Node* which,
        int num_retries)
{
    const std::string lua_scripts =
            "local n=redis.call('HLEN',KEYS[1]);"
            "local m=redis.call('HSETNX',KEYS[1],ARGV[1],ARGV[2]);"
            "if(n==0) then redis.call('EXPIRE',KEYS[1],ARGV[3]) end;return m;";
    std::vector<std::string> parameters(3);
    parameters[0] = field;
    parameters[1] = value;
    parameters[2] = int2string(expired_seconds);

    const RedisReplyHelper redis_reply = eval(key, lua_scripts, parameters, which, num_retries);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return 1 == redis_reply->integer;
    return true;
}

// Time complexity: O(1)
// HGET key field
bool CRedisClient::hget(
        const std::string& key,
        const std::string& field,
        std::string* value,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HGET");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(field);
    cmd_args.final();

    // Bulk string reply:
    // the value associated with field, or nil when field is not present in the hash or key does not exist.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
        return false;
    if (REDIS_REPLY_STRING == redis_reply->type)
        return get_value(redis_reply.get(), value);
    return true;
}

// Time complexity: O(1)
// HINCRBY key field increment
int64_t CRedisClient::hincrby(
        const std::string& key,
        const std::string& field,
        int64_t increment,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HINCRBY");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(field);
    cmd_args.add_arg(increment);
    cmd_args.final();

    // Integer reply: the value at field after the increment operation.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int64_t>(redis_reply->integer);
    return 0;
}

// hk Hash tag key
//
// ARGV[1] field
// ARGV[2] increment
// ARGV[3] unique id
// ARGV[4] expiration seconds
/*
local hk='{'..KEYS[1]..'}'..ARGV[3]
local ret=redis.call('SET',hk,'1','EX',ARGV[4],'NX')
if not ret then
  return nil
else
  return redis.call('HINCRBY',KEYS[1],ARGV[1],ARGV[2])
end
*/
bool CRedisClient::hincrby(
        const std::string& key,
        const std::string& field,
        int64_t increment,
        const std::string& uid,
        uint32_t expired_seconds,
        int64_t* newvalue,
        Node* which,
        int num_retries)
{
    int retries = 0;

HINCRBY_NOSCRIPT_RETRY: // No script to retry
    try
    {
        static const std::string hincrby_script =
                "local hk='{'..KEYS[1]..'}'..ARGV[3];"
                "local ret=redis.call('SET',hk,'1','EX',ARGV[4],'NX');"
                "if not ret then"
                "  return nil;"
                "else"
                "  return redis.call('HINCRBY',KEYS[1],ARGV[1],ARGV[2]);"
                "end";
        std::vector<std::string> parameters(4);
        RedisReplyHelper redis_reply;
        int64_t val = 0;

        parameters[0] = field;
        parameters[1] = int2string(increment);
        parameters[2] = uid;
        parameters[3] = int2string(expired_seconds);
        if (!_hincrby_shastr1.empty())
        {
            redis_reply = evalsha(key, _hincrby_shastr1, parameters, which, num_retries);
        }
        else
        {
            redis_reply = eval(key, hincrby_script, parameters, which, num_retries);
            _hincrby_shastr1 = strsha1(hincrby_script);
        }
        if (REDIS_REPLY_NIL == redis_reply->type) // Exists
            return false;
        else if (REDIS_REPLY_INTEGER == redis_reply->type) // Not Exists
            val = static_cast<int64_t>(redis_reply->integer);
        else if (REDIS_REPLY_STRING == redis_reply->type) // Not Exists
            string2int(redis_reply->str, redis_reply->len, &val);
        if (newvalue != NULL)
            *newvalue = val;
        return true;
    }
    catch (r3c::CRedisException& ex)
    {
        ++retries; // Only retry once
        if (retries>1 || !is_noscript_error(ex.errtype()))
        {
            throw;
        }
        else
        {
            _hincrby_shastr1.clear();
            goto HINCRBY_NOSCRIPT_RETRY;
        }
    }
}

void CRedisClient::hincrby(
        const std::string& key,
        const std::vector<std::pair<std::string, int64_t> >& increments,
        std::vector<int64_t>* newvalues,
        Node* which,
        int num_retries)
{
    hmincrby(key, increments, newvalues, which, num_retries);
}

void CRedisClient::hmincrby(
        const std::string& key,
        const std::vector<std::pair<std::string, int64_t> >& increments,
        std::vector<int64_t>* newvalues,
        Node* which,
        int num_retries)
{
    const std::string lua_scripts =
            "local j=1;local results={};"
            "for i=1,#ARGV,2 do local f=ARGV[i];"
            "local v=ARGV[i+1];"
            "results[j]=redis.call('HINCRBY',KEYS[1],f,v);j=j+1; end;"
            "return results;";
    std::vector<std::string> parameters(2*increments.size());
    for (std::vector<std::pair<std::string, int64_t> >::size_type i=0,j=0; i<increments.size(); ++i,j+=2)
    {
        const std::pair<std::string, int64_t>& increment = increments[i];
        parameters[j] = increment.first;
        parameters[j+1] = int2string(increment.second);
    }
    const RedisReplyHelper redis_reply = eval(key, lua_scripts, parameters, which, num_retries);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        get_values(redis_reply.get(), newvalues);
}

// hk Hash tag key
//
// ARGV[1] unique id
// ARGV[2] expiration seconds
// ARGV[3] field1
// ARGV[4] value1
// ARGV[5] field2
// ARGV[6] value2
// ... ...
/*
local hk='{'..KEYS[1]..'}'..ARGV[1]
local ret=redis.call('SET',hk,'1','EX',ARGV[2],'NX')
if not ret then
  return nil
else
  local results={}
  local j=1
  for i=3,#ARGV,2
  do
    local f=ARGV[i]
    local v=ARGV[i+1]
    results[j]=redis.call('HINCRBY',KEYS[1],f,v)
    j=j+1
  end
  return results
end
*/
bool CRedisClient::hmincrby(
        const std::string& key,
        const std::vector<std::pair<std::string, int64_t> >& increments,
        const std::string& uid,
        uint32_t expired_seconds,
        std::vector<int64_t>* newvalues,
        Node* which,
        int num_retries)
{
    int retries = 0;

HMINCRBY_NOSCRIPT_RETRY:
    try
    {
        static const std::string hmincrby_script =
                "local hk='{'..KEYS[1]..'}'..ARGV[1];"
                "local ret=redis.call('SET',hk,'1','EX',ARGV[2],'NX');"
                "if not ret then"
                "  return nil;"
                "else"
                "  local results={};"
                "  local j=1;"
                "  for i=3,#ARGV,2"
                "  do"
                "    local f=ARGV[i];"
                "    local v=ARGV[i+1];"
                "    results[j]=redis.call('HINCRBY',KEYS[1],f,v);"
                "    j=j+1;"
                "  end"
                "  return results;"
                "end";
        RedisReplyHelper redis_reply;
        std::vector<std::string> parameters(2+2*increments.size());
        parameters[0] = uid;
        parameters[1] = int2string(expired_seconds);
        for (std::vector<std::pair<std::string, int64_t> >::size_type i=0,j=2; i<increments.size(); ++i,j+=2)
        {
            const std::pair<std::string, int64_t>& increment = increments[i];
            parameters[j] = increment.first;
            parameters[j+1] = int2string(increment.second);
        }
        if (!_hmincrby_shastr1.empty())
        {
            redis_reply = evalsha(key, _hmincrby_shastr1, parameters, which, num_retries);
        }
        else
        {
            redis_reply = eval(key, hmincrby_script, parameters, which, num_retries);
            _hmincrby_shastr1 = strsha1(hmincrby_script);
        }
        if (REDIS_REPLY_NIL == redis_reply->type)
            return false;
        else if (REDIS_REPLY_ARRAY == redis_reply->type)
            get_values(redis_reply.get(), newvalues);
        return true;
    }
    catch (r3c::CRedisException& ex)
    {
        ++retries; // Only retry once
        if (retries>1 || !is_noscript_error(ex.errtype()))
        {
            throw;
        }
        else
        {
            _hmincrby_shastr1.clear();
            goto HMINCRBY_NOSCRIPT_RETRY;
        }
    }
}

void CRedisClient::hset(
        const std::string& key,
        const std::map<std::string, std::string>& map,
        Node* which,
        int num_retries)
{
    hmset(key, map, which, num_retries);
}

// Time complexity: O(N) where N is the number of fields being set.
// HMSET key field value [field value ...]
void CRedisClient::hmset(
        const std::string& key,
        const std::map<std::string, std::string>& map,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HMSET");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_args(map);
    cmd_args.final();

    // Simple string reply (REDIS_REPLY_STATUS)
    redis_command(false, num_retries, key, cmd_args, which);
}

int CRedisClient::hget(
        const std::string& key,
        const std::vector<std::string>& fields,
        std::map<std::string, std::string>* map,
        bool keep_null,
        Node* which,
        int num_retries)
{
    return hmget(key, fields, map, keep_null, which, num_retries);
}

// Time complexity: O(N) where N is the number of fields being requested.
// HMGET key field [field ...]
int CRedisClient::hmget(
        const std::string& key,
        const std::vector<std::string>& fields,
        std::map<std::string, std::string>* map,
        bool keep_null,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HMGET");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_args(fields);
    cmd_args.final();

    // Array reply:
    // list of values associated with the given fields, in the same order as they are requested.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
        return 0;
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), fields, keep_null, map);
    return 0;
}

// Time complexity: O(N) where N is the size of the hash.
// HGETALL key
int CRedisClient::hgetall(
        const std::string& key,
        std::map<std::string, std::string>* map,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HGETALL");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Array reply:
    // list of fields and their values stored in the hash, or an empty list when key does not exist.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
        return 0;
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), map);
    return 0;
}

// Time complexity: O(1)
// HSTRLEN key field
int CRedisClient::hstrlen(const std::string& key, const std::string& field, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HSTRLEN");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(field);
    cmd_args.final();

    // Integer reply:
    // the string length of the value associated with field,
    // or zero when field is not present in the hash or key does not exist at all.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
        return 0;
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Time complexity: O(N) where N is the size of the hash.
// HKEYS key
int CRedisClient::hkeys(const std::string& key, std::vector<std::string>* fields, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HKEYS");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Array reply:
    // list of fields in the hash, or an empty list when key does not exist.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
        return 0;
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), fields);
    return 0;
}

// Time complexity: O(N) where N is the size of the hash.
// HVALS key
int CRedisClient::hvals(const std::string& key, std::vector<std::string>* vals, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HVALS");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Array reply:
    // list of values in the hash, or an empty list when key does not exist.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
        return 0;
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), vals);
    return 0;
}

// Time complexity:
// O(1) for every call. O(N) for a complete iteration,
// including enough command calls for the cursor to return back to 0. N is the number of elements inside the collection..
//
// HSCAN key cursor [MATCH pattern] [COUNT count]
int64_t CRedisClient::hscan(
        const std::string& key,
        int64_t cursor,
        std::map<std::string, std::string>* map,
        Node* which,
        int num_retries)
{
    return hscan(key, cursor, std::string(""), 0, map, which, num_retries);
}

int64_t CRedisClient::hscan(
        const std::string& key,
        int64_t cursor, int count,
        std::map<std::string, std::string>* map,
        Node* which,
        int num_retries)
{
    return hscan(key, cursor, std::string(""), count, map, which, num_retries);
}

int64_t CRedisClient::hscan(
        const std::string& key,
        int64_t cursor, const std::string& pattern,
        std::map<std::string, std::string>* map,
        Node* which,
        int num_retries)
{
    return hscan(key, cursor, pattern, 0, map, which, num_retries);
}

int64_t CRedisClient::hscan(
        const std::string& key,
        int64_t cursor, const std::string& pattern, int count,
        std::map<std::string, std::string>* map,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("HSCAN");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(cursor);
    if (!pattern.empty())
    {
        cmd_args.add_arg("MATCH");
        cmd_args.add_arg(pattern);
    }
    if (count > 0)
    {
        cmd_args.add_arg("COUNT");
        cmd_args.add_arg(count);
    }
    cmd_args.final();

    // (gdb) p *redis_reply
    // $1 = {type = 2, integer = 0, len = 0, str = 0x0, elements = 2, element = 0x641450}
    // (gdb) p *redis_reply->element[0]
    // $2 = {type = 1, integer = 0, len = 1, str = 0x641820 "0", elements = 0, element = 0x0}
    // (gdb) p *redis_reply->element[1]
    // $3 = {type = 2, integer = 0, len = 0, str = 0x0, elements = 8, element = 0x6414b0}
    // (gdb) p *redis_reply->element[1]->element[0]
    // $4 = {type = 1, integer = 0, len = 2, str = 0x6418f0 "f1", elements = 0, element = 0x0}
    // (gdb) p *redis_reply->element[1]->element[1]
    // $5 = {type = 1, integer = 0, len = 2, str = 0x641950 "v1", elements = 0, element = 0x0}
    // (gdb) p *redis_reply->element[1]->element[2]
    // $6 = {type = 1, integer = 0, len = 2, str = 0x6419b0 "f2", elements = 0, element = 0x0}
    // (gdb) p *redis_reply->element[1]->element[3]
    // $7 = {type = 1, integer = 0, len = 2, str = 0x641a10 "v2", elements = 0, element = 0x0}
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
    {
        get_values(redis_reply->element[1], map);
        return static_cast<int64_t>(atoll(redis_reply->element[0]->str));
    }
    return 0;
}



//
// LIST
//

// Time complexity: O(1)
// LLEN key
int CRedisClient::llen(const std::string& key, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("LLEN");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Integer reply: the length of the list at key.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Time complexity: O(1)
// LPOP key
bool CRedisClient::lpop(const std::string& key, std::string* value, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("LPOP");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Bulk string reply: the value of the first element, or nil when key does not exist.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
        return false;
    if (REDIS_REPLY_STRING == redis_reply->type)
        return get_value(redis_reply.get(), value);
    return true; // MULTI & EXEC the type always is REDIS_REPLY_STATUS
}

// Time complexity: O(1)
// LPUSH key value [value ...]
int CRedisClient::lpush(
        const std::string& key,
        const std::string& value,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("LPUSH");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Integer reply: the length of the list after the push operations.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Time complexity: O(1)
// LPUSH key value [value ...]
int CRedisClient::lpush(
        const std::string& key,
        const std::vector<std::string>& values,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("LPUSH");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_args(values);
    cmd_args.final();

    // Integer reply: the length of the list after the push operations.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Inserts value at the head of the list stored at key, only if key already exists and holds a list.
// In contrary to LPUSH, no operation will be performed when key does not yet exist.
// Time complexity: O(1)
int CRedisClient::lpushx(const std::string& key, const std::string& value, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("LPUSHX");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Integer reply: the length of the list after the push operation.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Time complexity:
// O(S+N) where S is the distance of start offset from HEAD for small lists,
// from nearest end (HEAD or TAIL) for large lists;
// and N is the number of elements in the specified range.
//
// LRANGE key start stop
int CRedisClient::lrange(
        const std::string& key,
        int64_t start, int64_t end,
        std::vector<std::string>* values,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("LRANGE");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(start);
    cmd_args.add_arg(end);
    cmd_args.final();

    // Array reply: list of elements in the specified range.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), values);
    return 0;
}

// Time complexity:
// O(N) where N is the number of elements to be removed by the operation.
//
// LTRIM key start stop
void CRedisClient::ltrim(
        const std::string& key,
        int64_t start, int64_t end,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("LTRIM");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(start);
    cmd_args.add_arg(end);
    cmd_args.final();

    // Simple string reply (REDIS_REPLY_STATUS)
    redis_command(false, num_retries, key, cmd_args, which);
}

void CRedisClient::lset(const std::string& key, int index, const std::string& value, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("LSET");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(index);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Simple string reply (REDIS_REPLY_STATUS)
    // An error is returned for out of range indexes.
    redis_command(false, num_retries, key, cmd_args, which);
}

// Inserts value in the list stored at key either before or after the reference value pivot.
//
// Time complexity:
// O(N) where N is the number of elements to traverse before seeing the value pivot.
// This means that inserting somewhere on the left end on the list (head) can be considered O(1)
// and inserting somewhere on the right end (tail) is O(N).
int CRedisClient::linsert(const std::string& key, const std::string& pivot, const std::string& value, bool before, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("LINSERT");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    if (before)
        cmd_args.add_arg("BEFORE");
    else
        cmd_args.add_arg("AFTER");
    cmd_args.add_arg(pivot);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Integer reply:
    // the length of the list after the insert operation,
    // or -1 when the value pivot was not found.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Removes the first count occurrences of elements equal to value from the list stored at key.
// Time complexity:
// O(N) where N is the length of the list.
int CRedisClient::lrem(const std::string& key, int count, const std::string& value, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("LREM");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(count);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Integer reply: the number of removed elements.
    // Returns the number of removed elements.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Time complexity:
// O(N) where N is the number of elements to traverse to get to the element at index.
// This makes asking for the first or the last element of the list O(1).
bool CRedisClient::lindex(const std::string& key, int index, std::string* value, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("LINDEX");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(index);
    cmd_args.final();

    // Bulk string reply: the requested element, or nil when index is out of range.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
        return false;
    if (REDIS_REPLY_STRING == redis_reply->type)
        return get_value(redis_reply.get(), value);
    return true;
}

// Time complexity: O(1)
// RPOP key
bool CRedisClient::rpop(
        const std::string& key,
        std::string* value,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("RPOP");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Bulk string reply: the value of the last element, or nil when key does not exist.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
        return false;
    if (REDIS_REPLY_STRING == redis_reply->type)
        return get_value(redis_reply.get(), value);
    return true;
}

// Time complexity: O(1)
int CRedisClient::rpush(
        const std::string& key,
        const std::string& value,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("RPUSH");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Integer reply: the length of the list after the push operation.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Time complexity: O(1)
int CRedisClient::rpush(
        const std::string& key,
        const std::vector<std::string>& values,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("RPUSH");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_args(values);
    cmd_args.final();

    // Integer reply: the length of the list after the push operation.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Time complexity: O(1)
int CRedisClient::rpushx(
        const std::string& key,
        const std::string& value,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("RPUSHX");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Integer reply:
    // the length of the list after the push operation.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

//
// SET
//

// Time complexity:
//  O(1) for each element added, so O(N) to add N elements when the command is called with multiple arguments.
int CRedisClient::sadd(
        const std::string& key,
        const std::string& value,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SADD");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Integer reply: the number of elements that were added to the set,
    // not including all the elements already present into the set.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

//  O(1) for each element added, so O(N) to add N elements when the command is called with multiple arguments.
int CRedisClient::sadd(
        const std::string& key,
        const std::vector<std::string>& values,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SADD");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_args(values);
    cmd_args.final();

    // Integer reply: the number of elements that were added to the set,
    // not including all the elements already present into the set.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// O(1)
int CRedisClient::scard(const std::string& key, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SCARD");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Integer reply: the cardinality (number of elements) of the set, or 0 if key does not exist.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// O(1)
bool CRedisClient::sismember(
        const std::string& key,
        const std::string& value,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SISMEMBER");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Integer reply, specifically:
    // 1 if the element is a member of the set.
    // 0 if the element is not a member of the set, or if key does not exist.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return 1 == redis_reply->integer;
    return true;
}

// O(N) where N is the set cardinality.
int CRedisClient::smembers(
        const std::string& key,
        std::vector<std::string>* values,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SMEMBERS");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Array reply:
    // all elements of the set.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), values);
    return 0;
}

int CRedisClient::smembers(
        const std::string& key,
        std::set<std::string>* values,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SMEMBERS");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Array reply:
    // all elements of the set.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), values);
    return 0;
}

// Time complexity: O(1)
bool CRedisClient::spop(
        const std::string& key,
        std::string* value,
        Node* which,
        int num_retries)
{
    std::vector<std::string> values;
    const int n = spop(key, 1, &values, which, num_retries);
    if (n > 0)
    {
        *value = value[0];
    }
    return n > 0;
}

// Time complexity: O(1)
int CRedisClient::spop(
        const std::string& key,
        int count,
        std::vector<std::string>* values,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SPOP");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(count);
    cmd_args.final();

    // Bulk string reply:
    // the removed element, or nil when key does not exist.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), values);
    return 0;
}

// Time complexity: O(1)
bool CRedisClient::srandmember(
        const std::string& key,
        std::string* value,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SRANDMEMBER");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Bulk string reply:
    // without the additional count argument the command returns a Bulk Reply with the randomly selected element,
    // or nil when key does not exist.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
        return false;
    if (REDIS_REPLY_STRING == redis_reply->type)
        return get_value(redis_reply.get(), value);
    return true;
}

// Time complexity: O(N) where N is the absolute value of the passed count.
int CRedisClient::srandmember(
        const std::string& key,
        int count,
        std::vector<std::string>* values,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SRANDMEMBER");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(count);
    cmd_args.final();

    // Array reply:
    // when the additional count argument is passed the command returns an array of elements,
    // or an empty array when key does not exist.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), values);
    return 0;
}

// Time complexity: O(N) where N is the number of members to be removed.
int CRedisClient::srem(
        const std::string& key,
        const std::string& value,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SREM");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Integer reply: the number of members that were removed from the set,
    // not including non existing members.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

int CRedisClient::srem(
        const std::string& key,
        const std::vector<std::string>& values,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SREM");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_args(values);
    cmd_args.final();

    // Integer reply: the number of members that were removed from the set,
    // not including non existing members.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Time complexity:
// O(1) for every call.
// O(N) for a complete iteration, including enough command calls for the cursor to return back to 0.
// N is the number of elements inside the collection..
int64_t CRedisClient::sscan(
        const std::string& key,
        int64_t cursor,
        std::vector<std::string>* values,
        Node* which,
        int num_retries)
{
    return sscan(key, cursor, std::string(""), 0, values, which, num_retries);
}

int64_t CRedisClient::sscan(
        const std::string& key,
        int64_t cursor, int count,
        std::vector<std::string>* values,
        Node* which,
        int num_retries)
{
    return sscan(key, cursor, std::string(""), count, values, which, num_retries);
}

int64_t CRedisClient::sscan(
        const std::string& key,
        int64_t cursor, const std::string& pattern,
        std::vector<std::string>* values,
        Node* which,
        int num_retries)
{
    return sscan(key, cursor, pattern, 0, values, which, num_retries);
}

int64_t CRedisClient::sscan(
        const std::string& key,
        int64_t cursor, const std::string& pattern, int count,
        std::vector<std::string>* values,
        Node* which,
        int num_retries)
{
    // SSCAN key cursor [MATCH pattern] [COUNT count]
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SSCAN");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(cursor);
    if (!pattern.empty())
    {
        cmd_args.add_arg("MATCH");
        cmd_args.add_arg(pattern);
    }
    if (count > 0)
    {
        cmd_args.add_arg("COUNT");
        cmd_args.add_arg(count);
    }
    cmd_args.final();

    // (gdb) p *redis_reply
    // $3 = {type = 2, integer = 0, len = 0, str = 0x0, elements = 2, element = 0x641450}
    // (gdb) p *redis_reply->element[0]
    // $4 = {type = 1, integer = 0, len = 4, str = 0x641810 "6656", elements = 0, element = 0x0}
    // (gdb) p *redis_reply->element[1]
    // $5 = {type = 2, integer = 0, len = 0, str = 0x0, elements = 10, element = 0x641930}
    // (gdb) p *redis_reply->element[1]->element[0]
    // $6 = {type = 1, integer = 0, len = 5, str = 0x6419d0 "f1055", elements = 0, element = 0x0}
    // (gdb) p *redis_reply->element[1]->element[1]
    // $7 = {type = 1, integer = 0, len = 5, str = 0x641a30 "f6368", elements = 0, element = 0x0}
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
    {
        get_values(redis_reply->element[1], values);
        return static_cast<int64_t>(atoll(redis_reply->element[0]->str));
    }
    return 0;
}

int64_t CRedisClient::sscan(
        const std::string& key,
        int64_t cursor,
        const std::string& pattern,
        int count,
        std::set<std::string>* values,
        Node* which,
        int num_retries)
{
    // SSCAN key cursor [MATCH pattern] [COUNT count]
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SSCAN");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(cursor);
    if (!pattern.empty())
    {
        cmd_args.add_arg("MATCH");
        cmd_args.add_arg(pattern);
    }
    if (count > 0)
    {
        cmd_args.add_arg("COUNT");
        cmd_args.add_arg(count);
    }
    cmd_args.final();

    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
    {
        get_values(redis_reply->element[1], values);
        return static_cast<int64_t>(atoll(redis_reply->element[0]->str));
    }
    return 0;
}


// Copies all members of source keys to destinationkey.
// Time complexity: O(N) where N is the total number of elements in all given sets.
// Returns the number of members that were in resulting set.
int  CRedisClient::sunionstore(
    const std::string& destinationkey, 
    const std::vector<std::string>& keys, 
    Node* which, 
    int num_retries)
{
    
    std::vector<std::string> allKeys;
    allKeys=keys;
    allKeys.push_back(destinationkey);
    if(keys.size()<1){
        struct ErrorInfo errinfo;
        errinfo.errcode = ERROR_NOT_SUPPORT;
        errinfo.errmsg = "There must be minimum one key";
        THROW_REDIS_EXCEPTION(errinfo);        
    }
    else if (cluster_mode() && keys_crossslots(allKeys))
    {
        struct ErrorInfo errinfo;
        errinfo.errcode = ERROR_NOT_SUPPORT;
        errinfo.errmsg = "CROSSSLOT not supported in cluster mode";
        THROW_REDIS_EXCEPTION(errinfo);
    }
    else
    {    
        CommandArgs cmd_args;
        cmd_args.set_command("SUNIONSTORE");
        cmd_args.add_arg(cmd_args.get_command());
        cmd_args.add_arg(destinationkey);
        cmd_args.add_args(keys);
        cmd_args.final();

        // Integer reply, specifically:
        // Integer reply: the number of elements in the resulting set..
        const RedisReplyHelper redis_reply = redis_command(false, num_retries, destinationkey, cmd_args, which);
        if (REDIS_REPLY_INTEGER == redis_reply->type)
            return static_cast<int>(redis_reply->integer);
        return 0;
    }
}


//
// ZSET
//

// Time complexity:
// O(M*log(N)) with N being the number of elements in the sorted set and M the number of elements to be removed.
//
// ZREM key member [member ...]
int CRedisClient::zrem(
        const std::string& key,
        const std::string& field,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZREM");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(field);
    cmd_args.final();

    // Integer reply, specifically:
    // The number of members removed from the sorted set, not including non existing members.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Time complexity:
// O(M*log(N)) with N being the number of elements in the sorted set and M the number of elements to be removed.
int CRedisClient::zrem(
        const std::string& key,
        const std::vector<std::string>& fields,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZREM");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_args(fields);
    cmd_args.final();

    // Integer reply, specifically:
    // The number of members removed from the sorted set, not including non existing members.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Time complexity:
// O(log(N)) for each item added, where N is the number of elements in the sorted set.
//
// ZADD key [NX|XX] [CH] [INCR] score member [score member ...]
int CRedisClient::zadd(
        const std::string& key,
        const std::string& field,
        int64_t score,
        ZADDFLAG flag,
        Node* which,
        int num_retries)
{
    std::map<std::string, int64_t> map;
    map[field] = score;
    return zadd(key, map, flag, which, num_retries);
}

int CRedisClient::zadd(
        const std::string& key,
        const std::map<std::string, int64_t>& map,
        ZADDFLAG flag,
        Node* which,
        int num_retries)
{
    const std::string& flag_str = zaddflag2str(flag);
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZADD");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    if (!flag_str.empty())
    {
        cmd_args.add_arg(flag_str);
    }
    cmd_args.add_args(map, true);
    cmd_args.final();

    // Integer reply, specifically:
    // The number of elements added to the sorted sets, not including
    // elements already existing for which the score was updated.
    //
    // If the INCR option is specified, the return value will be Bulk string reply:
    // the new score of member (a double precision floating point number), represented as string.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Time complexity: O(1)
int64_t CRedisClient::zcard(const std::string& key, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZCARD");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Integer reply:
    // the cardinality (number of elements) of the sorted set, or 0 if key does not exist.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int64_t>(redis_reply->integer);
    return 0;
}

// Time complexity:
// O(log(N)) with N being the number of elements in the sorted set.
//
// ZCOUNT key min max
int64_t CRedisClient::zcount(
        const std::string& key,
        int64_t min, int64_t max ,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZCOUNT");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(min);
    cmd_args.add_arg(max);
    cmd_args.final();

    // Integer reply:
    // the number of elements in the specified score range.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int64_t>(redis_reply->integer);
    return 0;
}

// Time complexity:
// O(log(N)) where N is the number of elements in the sorted set.
//
// ZINCRBY key increment member
int64_t CRedisClient::zincrby(
        const std::string& key,
        const std::string& field, int64_t increment,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZINCRBY");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(increment);
    cmd_args.add_arg(field);
    cmd_args.final();

    // Bulk string reply:
    // the new score of member (a double precision floating point number), represented as string.
    //
    // (gdb) p *redis_reply._redis_reply
    // $4 = {type = 1, integer = 0, len = 2, str = 0x647860 "10", elements = 0, element = 0x0}
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_STRING == redis_reply->type)
        return static_cast<int64_t>(atoll(redis_reply->str));
    return 0;
}

// Time complexity:
// O(log(N)+M) with N being the number of elements in the sorted set and M the number of elements returned.
// ZRANGE key start stop [WITHSCORES]
//
// ZRANGE key start stop [WITHSCORES]
int CRedisClient::zrange(
        const std::string& key,
        int64_t start, int64_t end, bool withscores,
        std::vector<std::pair<std::string, int64_t> >* vec,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZRANGE");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(start);
    cmd_args.add_arg(end);
    if (withscores)
    {
        cmd_args.add_arg("WITHSCORES");
    }
    cmd_args.final();

    // Array reply:
    // list of elements in the specified range (optionally with their scores, in case the WITHSCORES option is given).
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), vec, withscores);
    return 0;
}

// Time complexity:
// O(log(N)+M) with N being the number of elements in the sorted set and M the number of elements returned.
//
// ZREVRANGE key start stop [WITHSCORES]
int CRedisClient::zrevrange(
        const std::string& key,
        int64_t start, int64_t end, bool withscores,
        std::vector<std::pair<std::string, int64_t> >* vec,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZREVRANGE");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(start);
    cmd_args.add_arg(end);
    if (withscores)
    {
        cmd_args.add_arg("WITHSCORES");
    }
    cmd_args.final();

    // Array reply:
    // list of elements in the specified range (optionally with their scores, in case the WITHSCORES option is given).
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), vec, withscores);
    return 0;
}

// Time complexity:
// O(log(N)+M) with N being the number of elements in the sorted set and M the number of elements being returned.
// If M is constant (e.g. always asking for the first 10 elements with LIMIT), you can consider it O(log(N)).
//
// ZRANGEBYSCORE key min max [WITHSCORES] [LIMIT offset count]
int CRedisClient::zrangebyscore(
        const std::string& key,
        int64_t min, int64_t max, bool withscores,
        std::vector<std::pair<std::string, int64_t> >* vec,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZRANGEBYSCORE");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(min);
    cmd_args.add_arg(max);
    if (withscores)
    {
        cmd_args.add_arg("WITHSCORES");
    }
    cmd_args.final();

    // Array reply:
    // list of elements in the specified score range (optionally with their scores).
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), vec, withscores);
    return 0;
}

// Time complexity:
// O(log(N)+M) with N being the number of elements in the sorted set and M the number of elements being returned.
// If M is constant (e.g. always asking for the first 10 elements with LIMIT), you can consider it O(log(N)).
//
// ZREVRANGEBYSCORE key max min [WITHSCORES] [LIMIT offset count]
int CRedisClient::zrevrangebyscore(
        const std::string& key,
        int64_t max, int64_t min, bool withscores,
        std::vector<std::pair<std::string, int64_t> >* vec,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZREVRANGEBYSCORE");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(max);
    cmd_args.add_arg(min);
    if (withscores)
    {
        cmd_args.add_arg("WITHSCORES");
    }
    cmd_args.final();

    // Array reply:
    // list of elements in the specified score range (optionally with their scores).
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), vec, withscores);
    return 0;
}

// ZRANGEBYSCORE key min max [WITHSCORES] [LIMIT offset count]
int CRedisClient::zrangebyscore(
        const std::string& key,
        int64_t min, int64_t max, int64_t offset, int64_t count, bool withscores,
        std::vector<std::pair<std::string, int64_t> >* vec,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZRANGEBYSCORE");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(min);
    cmd_args.add_arg(max);
    if (withscores)
    {
        cmd_args.add_arg("WITHSCORES");
    }
    cmd_args.add_arg("LIMIT");
    cmd_args.add_arg(offset);
    cmd_args.add_arg(count);
    cmd_args.final();

    // Array reply:
    // list of elements in the specified score range (optionally with their scores).
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), vec, withscores);
    return 0;
}

// ZREVRANGEBYSCORE key max min [WITHSCORES] [LIMIT offset count]
int CRedisClient::zrevrangebyscore(
        const std::string& key,
        int64_t max, int64_t min, int64_t offset, int64_t count, bool withscores,
        std::vector<std::pair<std::string, int64_t> >* vec,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZREVRANGEBYSCORE");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(max);
    cmd_args.add_arg(min);
    if (withscores)
    {
        cmd_args.add_arg("WITHSCORES");
    }
    cmd_args.add_arg("LIMIT");
    cmd_args.add_arg(offset);
    cmd_args.add_arg(count);
    cmd_args.final();

    // Array reply:
    // list of elements in the specified score range (optionally with their scores).
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
        return get_values(redis_reply.get(), vec, withscores);
    return 0;
}

// Time complexity:
// O(log(N)+M) with N being the number of elements in the sorted set and M the number of elements removed by the operation.
//
// ZREMRANGEBYRANK key start stop
int CRedisClient::zremrangebyrank(
        const std::string& key,
        int64_t start, int64_t end,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZREMRANGEBYRANK");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(start);
    cmd_args.add_arg(end);
    cmd_args.final();

    // Integer reply:
    // the number of elements removed.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// Time complexity: O(log(N))
//
// ZRANK key member
int CRedisClient::zrank(const std::string& key, const std::string& field, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZRANK");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(field);
    cmd_args.final();

    // If member exists in the sorted set, Integer reply: the rank of member.
    // If member does not exist in the sorted set or key does not exist, Bulk string reply: nil.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
    {
        return -1;
    }
    else
    {
        if (REDIS_REPLY_INTEGER == redis_reply->type)
            return static_cast<int>(redis_reply->integer);
        return 0;
    }
}

int CRedisClient::zrevrank(const std::string& key, const std::string& field, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZREVRANK");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(field);
    cmd_args.final();

    // If member exists in the sorted set, Integer reply: the rank of member.
    // If member does not exist in the sorted set or key does not exist, Bulk string reply: nil.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
    {
        return -1;
    }
    else
    {
        if (REDIS_REPLY_INTEGER == redis_reply->type)
            return static_cast<int>(redis_reply->integer);
        return 0;
    }
}

// Time complexity: O(1)
// ZSCORE key member
int64_t CRedisClient::zscore(const std::string& key, const std::string& field, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZSCORE");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(field);
    cmd_args.final();

    // Bulk string reply:
    // the score of member (a double precision floating point number), represented as string.
    // If member does not exist in the sorted set, or key does not exist, nil is returned.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_NIL == redis_reply->type)
    {
        return -1;
    }
    else
    {
        if (REDIS_REPLY_STRING == redis_reply->type)
            return static_cast<int64_t>(atoll(redis_reply->str));
        return 0;
    }
}

// Time complexity:
// O(1) for every call. O(N) for a complete iteration,
// including enough command calls for the cursor to return back to 0. N is the number of elements inside the collection..
//
// ZSCAN key cursor [MATCH pattern] [COUNT count]
int64_t CRedisClient::zscan(
        const std::string& key,
        int64_t cursor,
        std::vector<std::pair<std::string, int64_t> >* values,
        Node* which,
        int num_retries)
{
    return zscan(key, cursor, std::string(""), 0, values, which, num_retries);
}

int64_t CRedisClient::zscan(
        const std::string& key,
        int64_t cursor, int count,
        std::vector<std::pair<std::string, int64_t> >* values,
        Node* which,
        int num_retries)
{
    return zscan(key, cursor, std::string(""), count, values, which, num_retries);
}

int64_t CRedisClient::zscan(
        const std::string& key,
        int64_t cursor, const std::string& pattern,
        std::vector<std::pair<std::string, int64_t> >* values,
        Node* which,
        int num_retries)
{
    return zscan(key, cursor, pattern, 0, values, which, num_retries);
}

int64_t CRedisClient::zscan(
        const std::string& key,
        int64_t cursor, const std::string& pattern, int count,
        std::vector<std::pair<std::string, int64_t> >* values,
        Node* which,
        int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("ZSCAN");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(cursor);
    if (!pattern.empty())
    {
        cmd_args.add_arg("MATCH");
        cmd_args.add_arg(pattern);
    }
    if (count > 0)
    {
        cmd_args.add_arg("COUNT");
        cmd_args.add_arg(count);
    }
    cmd_args.final();

    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_ARRAY == redis_reply->type)
    {
        get_values(redis_reply->element[1], values, true);
        return static_cast<int64_t>(atoll(redis_reply->element[0]->str));
    }
    return 0;
}

//
// STREAM
//

// 从待处理条目列表（PEL）中移除待处理条目（ids），
// 因为一旦消息被成功处理，消费者组就不再需要跟踪它并记住消息的当前所有者
// Time complexity: O(1) for each message ID processed.
// XACK key group ID [ID ...]
int CRedisClient::xack(
        const std::string& key, const std::string& groupname,
        const std::vector<std::string>& ids,
        Node* which, int num_retries)
{
    std::string value;
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XACK");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(groupname);
    cmd_args.add_args(ids);

    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

int CRedisClient::xack(
        const std::string& key, const std::string& groupname,
        const std::string& id,
        Node* which, int num_retries)
{
    std::vector<std::string> ids(1);
    ids[0] = id;
    return xack(key, groupname, ids, which, num_retries);
}

// Time complexity: O(1)
// XADD key [MAXLEN [~|=] <count>] <ID or *> [field value] [field value] ...
std::string CRedisClient::xadd(
        const std::string& key, const std::string& id,
        const std::vector<FVPair>& values,
        int64_t maxlen, char c,
        Node* which, int num_retries)
{
    std::string value;
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XADD");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg("MAXLEN");
    cmd_args.add_arg(c);
    cmd_args.add_arg(maxlen);
    cmd_args.add_arg(id);
    cmd_args.add_args(values);
    cmd_args.final();

    // The command returns the number of messages successfully acknowledged.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    //if (REDIS_REPLY_STRING == redis_reply->type)
    (void)get_value(redis_reply.get(), &value);
    return value;
}

std::string CRedisClient::xadd(
        const std::string& key, const std::string& id,
        const std::vector<FVPair>& values,
        Node* which, int num_retries)
{
    std::string value;
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XADD");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(id);
    cmd_args.add_args(values);
    cmd_args.final();

    // The command returns the number of messages successfully acknowledged.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    //if (REDIS_REPLY_STRING == redis_reply->type)
    (void)get_value(redis_reply.get(), &value);
    return value;
}

// Time complexity: O(1) for all the subcommands, with the exception of the
// DESTROY subcommand which takes an additional O(M) time in order to delete
// the M entries inside the consumer group pending entries list (PEL).
//
// XGROUP CREATE key(topic) groupname id-or-$
void CRedisClient::xgroup_create(
        const std::string& key, const std::string& groupname, const std::string& id,
        bool mkstream,
        Node* which, int num_retries)
{
    // xgroup CREATE key groupname id-or-$
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XGROUP");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg("CREATE");
    cmd_args.add_arg(key);
    cmd_args.add_arg(groupname);
    cmd_args.add_arg(id);
    if (mkstream)
        cmd_args.add_arg("MKSTREAM");
    cmd_args.final();

    // If the specified consumer group already exists, the command returns a -BUSYGROUP error.
    // Otherwise the operation is performed and OK is returned.
    // There are no hard limits to the number of consumer groups you can associate to a given stream.
    //
    // Consumers in a consumer group are auto-created every time a new consumer name is mentioned by some command.
    //
    // 如果key不存在，则会报如下错误：
    // The XGROUP subcommand requires the key to exist.
    // Note that for CREATE you may want to use the MKSTREAM option to create an empty stream automatically.
    (void)redis_command(false, num_retries, key, cmd_args, which);
}

// To just remove a given consumer from a consumer group.
// Consumers in a consumer group are auto-created every time a new consumer name is mentioned by some command.
//
// XGROUP DESTROY key groupname
void CRedisClient::xgroup_destroy(const std::string& key, const std::string& groupname, Node* which, int num_retries)
{
    // xgroup DESTROY key groupname
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XGROUP");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg("DESTROY");
    cmd_args.add_arg(key);
    cmd_args.add_arg(groupname);
    cmd_args.final();

    // The consumer group will be destroyed even if there are active consumers
    // and pending messages, so make sure to call this command only when really needed.
    (void)redis_command(false, num_retries, key, cmd_args, which);
}

// XGROUP SETID key groupname id-or-$
void CRedisClient::xgroup_setid(const std::string& key, const std::string& id, Node* which, int num_retries)
{
    // xgroup SETID key id-or-$
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XGROUP");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg("SETID");
    cmd_args.add_arg(key);
    cmd_args.add_arg(id);
    cmd_args.final();

    (void)redis_command(false, num_retries, key, cmd_args, which);
}

// XGROUP DELCONSUMER key groupname consumername
void CRedisClient::xgroup_delconsumer(const std::string& key, const std::string& groupname, const std::string& consumername, Node* which, int num_retries)
{
    // xgroup DELCONSUMER key groupname consumername
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XGROUP");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg("DELCONSUMER");
    cmd_args.add_arg(key);
    cmd_args.add_arg(groupname);
    cmd_args.add_arg(consumername);
    cmd_args.final();

    (void)redis_command(false, num_retries, key, cmd_args, which);
}

// XREADGROUP命令是XREAD命令的特殊版本，支持消费者组
// 当使用XREADGROUP读取时，服务器将会记住某个给定的消息已经传递：
// 消息会被存储在消费者组内的待处理条目列表（PEL）中，即已送达但尚未确认的消息ID列表。
// 客户端必须使用XACK确认消息处理，以便从待处理条目列表中删除待处理条目，
// 还可以使用XPENDING命令检查待处理条目列表。
//
// 特殊ID“>”，意味着消费者希望只接收从未发送给任何其他消费者的消息，即只读取新的消息
//
// Reads more than one keys
//
// Available since 5.0.0.
// Time complexity: For each stream mentioned: O(M) with M being the number
// of elements returned. If M is constant (e.g. always asking for the first 10
// elements with COUNT), you can consider it O(1). On the other side when
// XREADGROUP blocks, XADD will pay the O(N) time in order to serve the N
// clients blocked on the stream getting new data.
//
// XREADGROUP GROUP group consumer [COUNT count] [BLOCK milliseconds] [NOACK] STREAMS key [key ...] ID [ID ...]
void CRedisClient::xreadgroup(
        const std::string& groupname, const std::string& consumername,
        const std::vector<std::string>& keys, const std::vector<std::string>& ids,
        int64_t count, int64_t block_milliseconds, bool noack,
        std::vector<Stream>* values,
        Node* which, int num_retries)
{
    if (keys.empty())
    {
        struct ErrorInfo errinfo;
        errinfo.errtype = "ERR";
        errinfo.errcode = ERROR_PARAMETER;
        errinfo.raw_errmsg = "wrong number of arguments for 'xreadgroup' command";
        errinfo.errmsg = format_string("[R3C_XPENDING][%s:%d] %s", __FILE__, __LINE__, errinfo.raw_errmsg.c_str());
        THROW_REDIS_EXCEPTION(errinfo);
    }
    else if (keys.size() != ids.size())
    {
        struct ErrorInfo errinfo;
        errinfo.errtype = "ERR";
        errinfo.errcode = ERROR_PARAMETER;
        errinfo.raw_errmsg = "unbalanced XREADGROUP list of streams: for each stream key an ID or '$' must be specified";
        errinfo.errmsg = format_string("[R3C_XPENDING][%s:%d] %s", __FILE__, __LINE__, errinfo.raw_errmsg.c_str());
        THROW_REDIS_EXCEPTION(errinfo);
    }
    else if (cluster_mode() && keys_crossslots(keys))
    {
        struct ErrorInfo errinfo;
        errinfo.errtype = "CROSSSLOT";
        errinfo.errcode = ERROR_PARAMETER;
        errinfo.raw_errmsg = "keys in request don't hash to the same slot";
        errinfo.errmsg = format_string("[R3C_XPENDING][%s:%d] %s", __FILE__, __LINE__, errinfo.raw_errmsg.c_str());
        THROW_REDIS_EXCEPTION(errinfo);
    }
    else
    {
        const std::string key = cluster_mode()? keys[0]: std::string("");
        std::string value;
        CommandArgs cmd_args;
        if (!key.empty())
            cmd_args.set_key(key);
        cmd_args.set_command("XREADGROUP");
        cmd_args.add_arg(cmd_args.get_command());
        cmd_args.add_arg("GROUP");
        cmd_args.add_arg(groupname);
        cmd_args.add_arg(consumername);
        cmd_args.add_arg("COUNT");
        cmd_args.add_arg(count);
        if (block_milliseconds >= 0) // timeout is negative
        {
            cmd_args.add_arg("BLOCK");
            cmd_args.add_arg(block_milliseconds);
        }
        if (noack)
        {
            cmd_args.add_arg("NOACK");
        }
        cmd_args.add_arg("STREAMS");
        cmd_args.add_args(keys);
        cmd_args.add_args(ids);
        cmd_args.final();

        // REDIS_REPLY_ARRAY
        const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
        get_values(redis_reply.get(), values);
    }
}

// Reads more than one keys
void CRedisClient::xreadgroup(
        const std::string& groupname, const std::string& consumername,
        const std::vector<std::string>& keys, const std::vector<std::string>& ids,
        int64_t count, bool noack,
        std::vector<Stream>* values,
        Node* which, int num_retries)
{
    const int64_t block_milliseconds = -1; // -1 means, no BLOCK argument given
    xreadgroup(groupname, consumername, keys, ids, count, block_milliseconds, noack, values, which, num_retries);
}

// Reads more than one keys
void CRedisClient::xreadgroup(
        const std::string& groupname, const std::string& consumername,
        const std::vector<std::string>& keys, const std::vector<std::string>& ids,
        bool noack,
        std::vector<Stream>* values,
        Node* which, int num_retries)
{
    const int64_t count = 0;
    const int64_t block_milliseconds = -1; // -1 means, no BLOCK argument given
    xreadgroup(groupname, consumername, keys, ids, count, block_milliseconds, noack, values, which, num_retries);
}

// Only read one key
void CRedisClient::xreadgroup(
        const std::string& groupname, const std::string& consumername,
        const std::string& key, const std::vector<std::string>& ids,
        int64_t count, int64_t block_milliseconds,
        bool noack,
        std::vector<StreamEntry>* values,
        Node* which, int num_retries)
{
    std::vector<Stream> streams;
    std::vector<std::string> keys(1);

    keys[0] = key;
    xreadgroup(groupname, consumername, keys, ids, count, block_milliseconds, noack, &streams, which, num_retries);
    if (!streams.empty())
        values->swap(streams[0].entries);
}

// Only read one key
void CRedisClient::xreadgroup(
        const std::string& groupname, const std::string& consumername,
        const std::string& key,
        int64_t count, int64_t block_milliseconds,
        bool noack,
        std::vector<StreamEntry>* values,
        Node* which, int num_retries)
{
    std::vector<Stream> streams;
    std::vector<std::string> keys(1);
    std::vector<std::string> ids(1);

    keys[0] = key;
    ids[0] = "$";
    xreadgroup(groupname, consumername, keys, ids, count, block_milliseconds, noack, &streams, which, num_retries);
    if (!streams.empty())
        values->swap(streams[0].entries);
}

// Reads more than one keys
// XREAD [COUNT count] [BLOCK milliseconds] STREAMS key [key ...] ID [ID ...]
void CRedisClient::xread(
        const std::vector<std::string>& keys, const std::vector<std::string>& ids,
        int64_t count, int64_t block_milliseconds,
        std::vector<Stream>* values, Node* which, int num_retries)
{
    if (keys.empty())
    {
        struct ErrorInfo errinfo;
        errinfo.errtype = "ERR";
        errinfo.errcode = ERROR_PARAMETER;
        errinfo.raw_errmsg = "wrong number of arguments for 'xread' command";
        errinfo.errmsg = format_string("[R3C_XPENDING][%s:%d] %s", __FILE__, __LINE__, errinfo.raw_errmsg.c_str());
        THROW_REDIS_EXCEPTION(errinfo);
    }
    else if (keys.size() != ids.size())
    {
        struct ErrorInfo errinfo;
        errinfo.errtype = "ERR";
        errinfo.errcode = ERROR_PARAMETER;
        errinfo.raw_errmsg = "unbalanced XREAD list of streams: for each stream key an ID or '$' must be specified";
        errinfo.errmsg = format_string("[R3C_XPENDING][%s:%d] %s", __FILE__, __LINE__, errinfo.raw_errmsg.c_str());
        THROW_REDIS_EXCEPTION(errinfo);
    }
    else if (cluster_mode() && keys_crossslots(keys))
    {
        struct ErrorInfo errinfo;
        errinfo.errtype = "CROSSSLOT";
        errinfo.errcode = ERROR_PARAMETER;
        errinfo.raw_errmsg = "keys in request don't hash to the same slot";
        errinfo.errmsg = format_string("[R3C_XPENDING][%s:%d] %s", __FILE__, __LINE__, errinfo.raw_errmsg.c_str());
        THROW_REDIS_EXCEPTION(errinfo);
    }
    else
    {
        const std::string key = cluster_mode()? keys[0]: std::string("");
        std::string value;
        CommandArgs cmd_args;
        if (!key.empty())
            cmd_args.set_key(key);
        cmd_args.set_command("XREAD");
        cmd_args.add_arg(cmd_args.get_command());
        cmd_args.add_arg("COUNT");
        cmd_args.add_arg(count);
        if (block_milliseconds >= 0) // timeout is negative
        {
            cmd_args.add_arg("BLOCK");
            cmd_args.add_arg(block_milliseconds);
        }
        cmd_args.add_arg("STREAMS");
        cmd_args.add_args(keys);
        cmd_args.add_args(ids);
        cmd_args.final();

        const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
        get_values(redis_reply.get(), values);
    }
}

// Reads more than one keys
void CRedisClient::xread(
        const std::vector<std::string>& keys, const std::vector<std::string>& ids,
        int64_t count,
        std::vector<Stream>* values, Node* which, int num_retries)
{
    const int64_t block_milliseconds = -1; // -1 means, no BLOCK argument given
    xread(keys, ids, count, block_milliseconds, values, which, num_retries);
}

// Reads more than one keys
void CRedisClient::xread(
        const std::vector<std::string>& keys, const std::vector<std::string>& ids,
        std::vector<Stream>* values, Node* which, int num_retries)
{
    const int64_t count = 0;
    const int64_t block_milliseconds = -1; // -1 means, no BLOCK argument given
    xread(keys, ids, count, block_milliseconds, values, which, num_retries);
}

// Only read one key
void CRedisClient::xread(
        const std::string& key, const std::vector<std::string>& ids,
        int64_t count, int64_t block_milliseconds,
        std::vector<StreamEntry>* values,
        Node* which, int num_retries)
{
    std::vector<Stream> streams;
    std::vector<std::string> keys(1);

    keys[0] = key;
    xread(keys, ids, count, block_milliseconds, &streams, which, num_retries);
    if (!streams.empty())
        values->swap(streams[0].entries);
}

// Only read one key
void CRedisClient::xread(
        const std::string& key,
        int64_t count, int64_t block_milliseconds,
        std::vector<StreamEntry>* values,
        Node* which, int num_retries)
{
    std::vector<Stream> streams;
    std::vector<std::string> keys(1);
    std::vector<std::string> ids(1);

    keys[0] = key;
    ids[0] = ">";
    xread(keys, ids, count, block_milliseconds, &streams, which, num_retries);
    if (!streams.empty())
        values->swap(streams[0].entries);
}

// XDEL <key> [<ID1> <ID2> ... <IDN>]
int CRedisClient::xdel(const std::string& key, const std::vector<std::string>& ids, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XDEL");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_args(ids);
    cmd_args.final();

    // Integer reply: the number of entries actually deleted.
    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

int CRedisClient::xdel(const std::string& key, const std::string& id, Node* which, int num_retries)
{
    std::vector<std::string> ids(1);
    ids[0] = id;
    return xdel(key, ids, which, num_retries);
}

// XTRIM key MAXLEN [~] count
int64_t CRedisClient::xtrim(const std::string& key, int64_t maxlen, char c, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XTRIM");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg("MAXLEN");
    cmd_args.add_arg(c);
    cmd_args.add_arg(maxlen);
    cmd_args.final();

    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

int64_t CRedisClient::xtrim(const std::string& key, int64_t maxlen, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XTRIM");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg("MAXLEN");
    cmd_args.add_arg(maxlen);
    cmd_args.final();

    const RedisReplyHelper redis_reply = redis_command(false, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// XLEN key
int64_t CRedisClient::xlen(const std::string& key, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XLEN");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.final();

    // Integer reply: the number of entries of the stream at key.
    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// XRANGE key start end [COUNT count]
void CRedisClient::xrange(
        const std::string& key,
        const std::string& start, const std::string& end, int64_t count,
        std::vector<StreamEntry>* values,
        Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XRANGE");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(start);
    cmd_args.add_arg(end);
    if (count >= 0)
    {
        cmd_args.add_arg("COUNT");
        cmd_args.add_arg(count);
    }
    cmd_args.final();

    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    get_values(redis_reply.get(), values);
}

void CRedisClient::xrange(
        const std::string& key,
        const std::string& start, const std::string& end,
        std::vector<StreamEntry>* values,
        Node* which, int num_retries)
{
    const int64_t count = -1;
    xrange(key, start, end, count, values, which, num_retries);
}

// XREVRANGE key end start [COUNT count]
void CRedisClient::xrevrange(
        const std::string& key,
        const std::string& end, const std::string& start, int64_t count,
        std::vector<StreamEntry>* values,
        Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XRANGE");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(end);
    cmd_args.add_arg(start);
    if (count >= 0)
    {
        cmd_args.add_arg("COUNT");
        cmd_args.add_arg(count);
    }
    cmd_args.final();

    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    get_values(redis_reply.get(), values);
}

void CRedisClient::xrevrange(
        const std::string& key,
        const std::string& end, const std::string& start,
        std::vector<StreamEntry>* values,
        Node* which, int num_retries)
{
    const int64_t count = -1;
    xrevrange(key, start, end, count, values, which, num_retries);
}

// PEL: Pending Entries List (待处理条目列表)
// XPENDING <key> <group> [<start> <stop> <count> [<consumer>]]
int CRedisClient::xpending(
        const std::string& key, const std::string& groupname,
        const std::string& start, const std::string& end, int count, const std::string& consumer,
        std::vector<struct DetailedPending>* pendings,
        Node* which, int num_retries)
{
    if (start.empty() || end.empty() || consumer.empty())
    {
        struct ErrorInfo errinfo;
        errinfo.errtype = "ERR";
        errinfo.raw_errmsg = "wrong number of arguments for 'xpending' command";
        errinfo.errmsg = format_string("[R3C_XPENDING][%s:%d] %s", __FILE__, __LINE__, errinfo.raw_errmsg.c_str());
        THROW_REDIS_EXCEPTION(errinfo);
    }
    else
    {
        CommandArgs cmd_args;
        cmd_args.set_key(key);
        cmd_args.set_command("XPENDING");
        cmd_args.add_arg(cmd_args.get_command());
        cmd_args.add_arg(key);
        cmd_args.add_arg(groupname);
        // 如果指定了start，则end和count也必须指定，否则会报错“syntax error”
        cmd_args.add_arg(start); // -
        cmd_args.add_arg(end); // +
        cmd_args.add_arg(count);
        if (!consumer.empty())
            cmd_args.add_arg(consumer);
        cmd_args.final();

        // The command returns data in different format depending on the way it is called,
        // as previously explained in this page. However the reply is always an array of items.
        const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
        return get_values(redis_reply.get(), pendings);
    }
}

int CRedisClient::xpending(
        const std::string& key, const std::string& groupname,
        const std::string& start, const std::string& end, int count,
        std::vector<struct DetailedPending>* pendings,
        Node* which, int num_retries)
{
    const std::string consumer;
    return xpending(key, groupname, start, end, count, consumer, pendings, which, num_retries);
}

int CRedisClient::xpending(
        const std::string& key, const std::string& groupname,
        struct GroupPending* groups,
        Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XPENDING");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(groupname);
    cmd_args.final();

    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    return get_values(redis_reply.get(), groups);
}

// Gets ownership of one or multiple messages in the Pending Entries List
// of a given stream consumer group.
//
// If the message ID (among the specified ones) exists, and its idle
// time greater or equal to <min-idle-time>, then the message new owner
// becomes the specified <consumer>. If the minimum idle time specified
// is zero, messages are claimed regardless of their idle time.
//
// All the messages that cannot be found inside the pending entries list
// are ignored, but in case the FORCE option is used. In that case we
// create the NACK (representing a not yet acknowledged message) entry in
// the consumer group PEL.
//
// This command creates the consumer as side effect if it does not yet
// exists. Moreover the command reset the idle time of the message to 0,
// even if by using the IDLE or TIME options, the user can control the
// new idle time.
//
// IDLE(IDLE和TIME为二选一关系):
// Set the idle time (last time it was delivered) of the message.
// If IDLE is not specified, an IDLE of 0 is assumed, that is,
// the time count is reset because the message has now a new
// owner trying to process it.
//
// TIME(IDLE和TIME为二选一关系):
// This is the same as IDLE but instead of a relative amount of
// milliseconds, it sets the idle time to a specific unix time
// (in milliseconds). This is useful in order to rewrite the AOF
// file generating XCLAIM commands.
//
// RETRYCOUNT:
// Set the retry counter to the specified value. This counter is
// incremented every time a message is delivered again. Normally
// XCLAIM does not alter this counter, which is just served to clients
// when the XPENDING command is called: this way clients can detect
// anomalies, like messages that are never processed for some reason
// after a big number of delivery attempts.
//
// FORCE:
// Creates the pending message entry in the PEL even if certain
// specified IDs are not already in the PEL assigned to a different
// client. However the message must be exist in the stream, otherwise
// the IDs of non existing messages are ignored.
//
// JUSTID:
// Return just an array of IDs of messages successfully claimed,
// without returning the actual message.
//
// LASTID:
// Update the consumer group last ID with the specified ID if the
// current last ID is smaller than the provided one.
// This is used for replication / AOF, so that when we read from a
// consumer group, the XCLAIM that gets propagated to give ownership
// to the consumer, is also used in order to update the group current
// ID.
//
// XCLAIM <key> <group> <consumer> <min-idle-time> <ID-1> <ID-2>
//        [IDLE <milliseconds>] [TIME <mstime>] [RETRYCOUNT <count>] [FORCE] [JUSTID]
// 改变待处理消息的所有权， 因此新的所有者是在命令参数中指定的消费者
void CRedisClient::xclaim(
        const std::string& key, const std::string& groupname, const std::string& consumer,
        int64_t minidle, const std::vector<std::string>& ids,
        int64_t idletime, int64_t unixtime, int64_t retrycount, bool force,
        std::vector<StreamEntry>* values,
        Node* which, int num_retries)
{
    if (ids.empty())
    {
        struct ErrorInfo errinfo;
        errinfo.errtype = "ERR";
        errinfo.raw_errmsg = "wrong number of arguments for 'xclaim' command";
        errinfo.errmsg = format_string("[R3C_XPENDING][%s:%d] %s", __FILE__, __LINE__, errinfo.raw_errmsg.c_str());
        THROW_REDIS_EXCEPTION(errinfo);
    }
    else
    {
        CommandArgs cmd_args;
        cmd_args.set_key(key);
        cmd_args.set_command("XCLAIM");
        cmd_args.add_arg(cmd_args.get_command());
        cmd_args.add_arg(key);
        cmd_args.add_arg(groupname);
        cmd_args.add_arg(consumer);
        if (minidle >= 0)
        {
            cmd_args.add_arg(minidle);
        }
        if (!ids.empty())
        {
            cmd_args.add_args(ids);
        }
        if (idletime >= 0)
        {
            cmd_args.add_arg("IDLE");
            cmd_args.add_arg(idletime);
        }
        if (unixtime >= 0)
        {
            cmd_args.add_arg("TIME");
            cmd_args.add_arg(unixtime);
        }
        if (retrycount >= 0)
        {
            cmd_args.add_arg(retrycount);
        }
        if (force)
        {
            cmd_args.add_arg("FORCE");
        }
        //cmd_args.add_arg("JUSTID");
        cmd_args.final();

        // Returns all the messages successfully claimed, in the same format as XRANGE.
        // However if the JUSTID option was specified,
        // only the message IDs are reported, without including the actual message.
        const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
        get_values(redis_reply.get(), values);
    }
}

void CRedisClient::xclaim(
        const std::string& key, const std::string& groupname, const std::string& consumer,
        int64_t minidle, const std::vector<std::string>& ids,
        std::vector<StreamEntry>* values,
        Node* which, int num_retries)
{
    const int64_t idletime = -1;
    const int64_t unixtime = -1;
    const int64_t retrycount = -1;
    bool force = false;
    xclaim(key, groupname, consumer, minidle, ids, idletime, unixtime, retrycount, force, values, which, num_retries);
}

void CRedisClient::xclaim(
        const std::string& key, const std::string& groupname, const std::string& consumer,
        int64_t minidle, const std::vector<std::string>& ids,
        int64_t idletime, int64_t unixtime, int64_t retrycount,
        bool force,
        std::vector<std::string>* values,
        Node* which, int num_retries)
{
    if (ids.empty())
    {
        struct ErrorInfo errinfo;
        errinfo.errtype = "ERR";
        errinfo.raw_errmsg = "wrong number of arguments for 'xclaim' command";
        errinfo.errmsg = format_string("[R3C_XPENDING][%s:%d] %s", __FILE__, __LINE__, errinfo.raw_errmsg.c_str());
        THROW_REDIS_EXCEPTION(errinfo);
    }
    else
    {
        CommandArgs cmd_args;
        cmd_args.set_key(key);
        cmd_args.set_command("XCLAIM");
        cmd_args.add_arg(cmd_args.get_command());
        cmd_args.add_arg(key);
        cmd_args.add_arg(groupname);
        cmd_args.add_arg(consumer);
        if (minidle >= 0)
        {
            cmd_args.add_arg(minidle);
        }
        if (!ids.empty())
        {
            cmd_args.add_args(ids);
        }
        if (idletime >= 0)
        {
            cmd_args.add_arg("IDLE");
            cmd_args.add_arg(idletime);
        }
        if (unixtime >= 0)
        {
            cmd_args.add_arg("TIME");
            cmd_args.add_arg(unixtime);
        }
        if (retrycount >= 0)
        {
            cmd_args.add_arg(retrycount);
        }
        if (force)
        {
            cmd_args.add_arg("FORCE");
        }
        cmd_args.add_arg("JUSTID");
        cmd_args.final();

        // Returns all the messages successfully claimed, in the same format as XRANGE.
        // However if the JUSTID option was specified,
        // only the message IDs are reported, without including the actual message.
        const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
        get_values(redis_reply.get(), values);
    }
}

void CRedisClient::xclaim(
        const std::string& key, const std::string& groupname, const std::string& consumer,
        int64_t minidle, const std::vector<std::string>& ids,
        std::vector<std::string>* values,
        Node* which, int num_retries)
{
    const int64_t idletime = -1;
    const int64_t unixtime = -1;
    const int64_t retrycount = -1;
    bool force = false;
    xclaim(key, groupname, consumer, minidle, ids, idletime, unixtime, retrycount, force, values, which, num_retries);
}

// Show consumer groups of group <groupname>.
// XINFO CONSUMERS <key> <group>
int CRedisClient::xinfo_consumers(
        const std::string& key, const std::string& groupname,
        std::vector<struct ConsumerInfo>* infos,
        Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XINFO");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg("CONSUMERS");
    cmd_args.add_arg(key);
    cmd_args.add_arg(groupname);
    cmd_args.final();

    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    return get_values(redis_reply.get(), infos);
}

// Show the stream consumer groups.
// XINFO GROUPS <key>
int CRedisClient::xinfo_groups(
        const std::string& key,
        std::vector<struct GroupInfo>* infos,
        Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XINFO");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg("GROUPS");
    cmd_args.add_arg(key);
    cmd_args.final();

    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    return get_values(redis_reply.get(), infos);
}

// Show information about the stream.
// XINFO STREAM <key>
void CRedisClient::xinfo_stream(
        const std::string& key,
        struct StreamInfo* info,
        Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("XINFO");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg("STREAM");
    cmd_args.add_arg(key);
    cmd_args.final();

    const RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    get_value(redis_reply.get(), info);
}

// SETBIT key offset value
// Time complexity: O(1)
#if !defined(setbit)
void CRedisClient::setbit(const std::string& key, uint32_t offset, uint32_t value, Node* which, int num_retries)
#else
#undef setbit
void CRedisClient::setbit(const std::string& key, uint32_t offset, uint32_t value, Node* which, int num_retries)
#endif // setbit
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("SETBIT");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(offset);
    cmd_args.add_arg(value);
    cmd_args.final();

    // Integer reply:
    // the original value of the bit on the specified offset.
    redis_command(false, num_retries, key, cmd_args, which);
}

// GETBIT key offset 
// Time complexity: O(1)
int CRedisClient::getbit(const std::string& key, uint32_t offset, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("GETBIT");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(offset);
    cmd_args.final();

    // Integer reply:
    // the value of the bit on the offset, return 0 if key does not exist.
    RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// BITCOUNT key [start end]
// Time complexity: O(N)
int CRedisClient::bitcount(const std::string& key, int32_t start, int32_t end, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("BITCOUNT");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);       
    cmd_args.add_arg(start);
    cmd_args.add_arg(end);
    cmd_args.final();
    
    // Integer reply:
    // the number of bits set to 1.
    RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

// BITPOS key bit [start] [end]
// Time complexity: O(N)
int CRedisClient::bitpos(const std::string& key, uint8_t bit, int32_t start, int32_t end, Node* which, int num_retries)
{
    CommandArgs cmd_args;
    cmd_args.set_key(key);
    cmd_args.set_command("BITPOS");
    cmd_args.add_arg(cmd_args.get_command());
    cmd_args.add_arg(key);
    cmd_args.add_arg(bit);        
    cmd_args.add_arg(start);
    cmd_args.add_arg(end);
    cmd_args.final();
    
    // Integer reply:
    // the position of the first bit in the bitmap whose value is bit.
    RedisReplyHelper redis_reply = redis_command(true, num_retries, key, cmd_args, which);
    if (REDIS_REPLY_INTEGER == redis_reply->type)
        return static_cast<int>(redis_reply->integer);
    return 0;
}

////////////////////////////////////////////////////////////////////////////////

const RedisReplyHelper
CRedisClient::redis_command(
        bool readonly, int num_retries,
        const std::string& key,
        const CommandArgs& command_args,
        Node* which)
{
    Node node;
    Node* ask_node = NULL;
    RedisReplyHelper redis_reply;
    struct ErrorInfo errinfo;

    if (cluster_mode() && key.empty())
    {
        // 集群模式必须指定key
        errinfo.errcode = ERROR_ZERO_KEY;
        errinfo.raw_errmsg = format_string("[%s] key is empty in cluster node", command_args.get_command().c_str());
        errinfo.errmsg = format_string("[R3C_CMD][%s:%d] %s", __FILE__, __LINE__, errinfo.raw_errmsg.c_str());
        if (_enable_error_log)
            (*g_error_log)("%s\n", errinfo.errmsg.c_str());
        THROW_REDIS_EXCEPTION(errinfo);
    }
    for (int loop_counter=0;;++loop_counter)
    {
        const int slot = cluster_mode()? get_key_slot(&key): -1;
        CRedisNode* redis_node = get_redis_node(slot, readonly, ask_node, &errinfo);
        HandleResult errcode;

        if (NULL == redis_node)
        {
            node.first.clear(); node.second = 0;
        }
        else
        {
            node = redis_node->get_node();
        }
        if (which != NULL)
        {
            *which = node;
        }
        if (0==loop_counter && _command_monitor!= NULL)
        {
            _command_monitor->before_execute(node, command_args.get_command(), command_args, readonly);
        }
        if (NULL == redis_node)
        {
            errinfo.errcode = ERROR_NO_ANY_NODE;
            errinfo.raw_errmsg = format_string("[%s][%s][%s:%d] no any node", command_args.get_command().c_str(), get_mode_str(), node.first.c_str(), node.second);
            errinfo.errmsg = format_string("[R3C_CMD][%s:%d] %s", __FILE__, __LINE__, errinfo.raw_errmsg.c_str());
            if (_enable_error_log)
                (*g_error_log)("[NO_ANY_NODE] %s\n", errinfo.errmsg.c_str());
            break; // 没有任何master
        }
        if (NULL == redis_node->get_redis_context())
        {
            // 连接master不成功
            errcode = HR_RECONN_UNCOND;
        }
        else
        {
            // When a slot is set as MIGRATING, the node will accept all queries that are about this hash slot,
            // but only if the key in question exists,
            // otherwise the query is forwarded using a -ASK redirection to the node that is target of the migration.
            // 当一个槽状态为 MIGRATING时，原来负责该哈希槽的节点仍会接受所有跟这个哈希槽有关的请求，但只处理仍然在该节点上的键，
            // 否则类似于MOVED，返回ASK重定向到目标节点。
            //
            // When a slot is set as IMPORTING, the node will accept all queries that are about this hash slot,
            // but only if the request is preceded by an ASKING command.
            // If the ASKING command was not given by the client, the query is redirected to the real hash slot owner via a -MOVED redirection error,
            // as would happen normally.
            // 当一个槽状态为 IMPORTING时，只有在接受到 ASKING命令之后节点才会接受所有查询这个哈希槽的请求，
            // 如果客户端一直没有发送 ASKING命令，那么查询都会通过MOVED重定向错误转发到真正处理这个哈希槽的节点那里。
            if (ask_node != NULL) (void)redisCommand(redis_node->get_redis_context(), "ASKING");
            redis_reply = (redisReply*)redisCommandArgv(
                    redis_node->get_redis_context(),
                    command_args.get_argc(), command_args.get_argv(), command_args.get_argvlen());
#if R3C_TEST // for test
            debug_redis_reply(command_args.get_command(), redis_reply.get());
#endif // R3C_TEST==1
            if (!redis_reply)
                errcode = handle_redis_command_error(redis_node, command_args, &errinfo);
            else
                errcode = handle_redis_reply(redis_node, command_args, redis_reply.get(), &errinfo);
        }

        ask_node = NULL;
        if (HR_SUCCESS == errcode)
        {
            // 成功立即返回
            if (_command_monitor!=NULL)
                _command_monitor->after_execute(0, node, command_args.get_command(), redis_reply.get());
            return redis_reply;
        }
        else if (HR_ERROR == errcode)
        {
            // 不需要重试的错误立即返回，比如：EVAL命令语法错误
            if (_enable_debug_log)
            {
                (*g_debug_log)("[NOTRETRY][%s:%d][%s][%s:%d] loop: %d\n",
                        __FILE__, __LINE__, get_mode_str(),
                        redis_node->get_node().first.c_str(), redis_node->get_node().second, loop_counter);
            }
            break;
        }
        else if (HR_RECONN_COND == errcode || HR_RECONN_UNCOND == errcode)
        {
            // 连接问题，先调用close关闭连接（调用get_redis_node时就会执行重连接）
            redis_node->close();
        }
        else if (HR_REDIRECT == errcode)
        {
            if (!parse_moved_string(redis_reply->str, &node))
            {
                if (_enable_error_log)
                {
                    (*g_error_log)("[PARSE_MOVED][%s:%d][%s][%s:%d] node string error: %s\n",
                            __FILE__, __LINE__, get_mode_str(),
                            redis_node->get_node().first.c_str(), redis_node->get_node().second,
                            redis_reply->str);
                }
                break;
            }
            else
            {
                ask_node = &node;
                if (loop_counter <= 2)
                    continue;
                if (_enable_debug_log)
                {
                    (*g_debug_log)("[REDIRECT][%s:%d][%s][%s:%d] retries more than %d\n",
                            __FILE__, __LINE__, get_mode_str(),
                            redis_node->get_node().first.c_str(), redis_node->get_node().second, loop_counter);
                }
                break;
            }
        }

        if (HR_RECONN_UNCOND == errcode)
        {
            // 保持至少重试一次（前提是先重新建立好连接）
            if (loop_counter>num_retries && loop_counter>0)
            {
                if (_enable_debug_log)
                {
                    (*g_debug_log)("[NOTRECONN][%s:%d][%s][%s:%d] retries more than %d\n",
                            __FILE__, __LINE__, get_mode_str(),
                            redis_node->get_node().first.c_str(), redis_node->get_node().second, num_retries);
                }
                break;
            }
        }
        else if (HR_RETRY_UNCOND == errcode)
        {
            // 一般replica切换成master需要几秒钟，
            // 所以需要保证有足够的重试次数
            if (loop_counter > num_retries)
            {
                if (_enable_debug_log)
                {
                    (*g_debug_log)("[RETRY_UNCOND][%s:%d][%s][%s:%d] retries more than %d\n",
                            __FILE__, __LINE__, get_mode_str(),
                            redis_node->get_node().first.c_str(), redis_node->get_node().second, num_retries);
                }
                break;
            }
        }
        else if (loop_counter>=num_retries)
        {
            if (_enable_debug_log)
            {
                (*g_debug_log)("[OVERRETRY][%s:%d][%s][%s:%d] retries more than %d\n",
                        __FILE__, __LINE__, get_mode_str(),
                        redis_node->get_node().first.c_str(), redis_node->get_node().second, num_retries);
            }
            break;
        }

        // 控制重试频率，以增强重试成功率
        if (HR_RETRY_UNCOND == errcode || HR_RECONN_UNCOND == errcode)
        {
            const int retry_sleep_milliseconds = get_retry_sleep_milliseconds(loop_counter);
            if (retry_sleep_milliseconds > 0)
                millisleep(retry_sleep_milliseconds);
        }
        if (cluster_mode() && redis_node->need_refresh_master())
        {
            // 单机模式下走到这会导致没法重连接，
            // 因为_redis_master_nodes被清空了。
            if (HR_RECONN_COND==errcode || HR_RECONN_UNCOND==errcode)
                refresh_master_node_table(&errinfo, &node);
            else
                refresh_master_node_table(&errinfo, NULL);
        }
    }

    // 错误以异常方式抛出
    if (_command_monitor!=NULL)
        _command_monitor->after_execute(1, node, command_args.get_command(), redis_reply.get());
    THROW_REDIS_EXCEPTION_WITH_NODE_AND_COMMAND(errinfo, node.first, node.second, command_args.get_command(), command_args.get_key());
}

CRedisClient::HandleResult
CRedisClient::handle_redis_command_error(
        CRedisNode* redis_node,
        const CommandArgs& command_args,
        struct ErrorInfo* errinfo)
{
    redisContext* redis_context = redis_node->get_redis_context();

    // REDIS_ERR_EOF (call read() return 0):
    // redis_context->err(3)
    // redis_context->errstr("Server closed the connection")
    //
    // REDIS_ERR_IO (call read() return -1):
    // redis_context->err(1)
    // redis_context->errstr("Bad file descriptor")
    //
    // REDIS_ERR_IO means there was an I/O error
    // and you should use the "errno" variable to find out what is wrong.
    // For other values, the "errstr" field will hold a description.	
#ifdef _MSC_VER
	char errmsg[512];
	strerror_s(errmsg, 512, errno);

	const int redis_errcode = redis_context->err;
	errinfo->errcode = errno;
	errinfo->raw_errmsg = format_string("[%s] (%d/%d)%s (%s)",
		redis_node->str().c_str(), redis_errcode, errinfo->errcode, redis_context->errstr, errmsg);
#else
		const int redis_errcode = redis_context->err;
		errinfo->errcode = errno;
		errinfo->raw_errmsg = format_string("[%s] (%d/%d)%s (%s)",
            redis_node->str().c_str(), redis_errcode, errinfo->errcode, redis_context->errstr, strerror(errinfo->errcode));
#endif
    errinfo->errmsg = format_string("[R3C_CMD_ERROR][%s:%d][%s] %s",
            __FILE__, __LINE__, command_args.get_command().c_str(), errinfo->raw_errmsg.c_str());
    if (_enable_error_log)
    {
        const unsigned int conn_errors = redis_node->get_conn_errors();
        if (conn_errors==0 || conn_errors%10==0) // Reduce duplicate error logs
            (*g_error_log)("%s\n", errinfo->errmsg.c_str());
    }

    // REDIS_ERR_IO:
    // There was an I/O error while creating the connection, trying to write
    // to the socket or read from the socket. If you included `errno.h` in your
    // application, you can use the global `errno` variable to find out what is wrong.
    //
    // REDIS_ERR_EOF:
    // The server closed the connection which resulted in an empty read.
    //
    // REDIS_ERR_PROTOCOL:
    // There was an error while parsing the protocol.
    //
    // REDIS_ERR_OTHER:
    // Any other error. Currently, it is only used when a specified hostname to connect to cannot be resolved.
    if ((redis_errcode != REDIS_ERR_IO) &&
        (redis_errcode != REDIS_ERR_EOF))
    {
        return HR_ERROR; // Not retry
    }
    else if (REDIS_ERR_EOF == redis_errcode)
    {
        redis_node->inc_conn_errors();
        return HR_RECONN_COND; // Retry unconditionally and reconnect
    }
    else
    {
        // $ redis-cli -p 1386 cluster nodes | grep master
        // 49cadd758538f821b922738fd000b5a16ef64fc7 127.0.0.1:1384@11384 master - 0 1546317629187 14 connected 10923-16383
        // 95ce22507d469ae84cb760505bba4be0283c5468 127.0.0.1:1381@11381 master - 0 1546317630190 1 connected 0-5460
        // a27a1ce7f8c5c5f79a1d09227eb80b73919ec795 127.0.0.1:1383@11383 master,fail - 1546317518930 1546317515000 12 connected
        // 03c4ada274ac137c710f3d514700c2e94649c131 127.0.0.1:1382@11382 master - 0 1546317631191 2 connected 5461-10922
        // 错误期间，可能发生了主备切换，因此光重连接是不够的，
        // 更严重的是，该master可能一直连接超时，比如进程被SIGSTOP了，
        // 因此重试几次后，应当重刷新master
        redis_node->inc_conn_errors();
        return HR_RECONN_COND; // Retry conditionally
    }
}

CRedisClient::HandleResult
CRedisClient::handle_redis_reply(
        CRedisNode* redis_node,
        const CommandArgs& command_args,
        const redisReply* redis_reply,
        struct ErrorInfo* errinfo)
{
    redis_node->reset_conn_errors();

    if (redis_reply->type != REDIS_REPLY_ERROR)
        return HR_SUCCESS;
    else
        return handle_redis_replay_error(redis_node, command_args, redis_reply, errinfo);
}

CRedisClient::HandleResult
CRedisClient::handle_redis_replay_error(
        CRedisNode* redis_node,
        const CommandArgs& command_args,
        const redisReply* redis_reply,
        struct ErrorInfo* errinfo)
{
    // ASK
    // NOAUTH Authentication required
    // MOVED 6474 127.0.0.1:6380
    // WRONGTYPE Operation against a key holding the wrong kind of value
    // CLUSTERDOWN The cluster is down
    // NOSCRIPT No matching script. Please use EVAL.
    //
    // ERR Error running script (call to f_64b2246244d60088e7351656c216c00cb1d7d2fd) : @user_script:1: @user_script: 1: Lua script attempted to access a non local key in a cluster node
    extract_errtype(redis_reply, &errinfo->errtype);
    errinfo->errcode = ERROR_COMMAND;
    errinfo->raw_errmsg = format_string("[%s] %s", redis_node->str().c_str(), redis_reply->str);
    errinfo->errmsg = format_string("[R3C_REPLAY_ERROR][%s:%d][%s] %s", __FILE__, __LINE__, command_args.get_command().c_str(), errinfo->raw_errmsg.c_str());
    if (_enable_error_log)
        (*g_error_log)("%s\n", errinfo->errmsg.c_str());

    // EVAL不会返回“MOVED”的错误，需特别处理
    if (is_clusterdown_error(errinfo->errtype))
    {
        return HR_RETRY_UNCOND;
    }
    else if (is_ask_error(errinfo->errtype))
    {
        // ASK 6474 127.0.0.1:6380
        //
        // 如下所示，如果此时向10.212.2.72:6379请求slot为14148的keys时，将得到：
        // ASK 6474 10.212.2.71:6381
        // 其中ec19be9a50b5416999ac0305c744d9b6c957c18d为10.212.2.71:6381的NodeId
        // e008649f6f8340a495fc860f7a9a8155f91fcb93 10.212.2.72:6379@16379 myself,master - 0 1547374698000 35 connected 5461-10922 14148 [14148->-ec19be9a50b5416999ac0305c744d9b6c957c18d]
        return HR_REDIRECT;
    }
    else if (is_moved_error(errinfo->errtype))
    {
        // MOVED 6474 127.0.0.1:6380
        //
        //Node ask_node;
        //parse_moved_string(redis_reply->str, &ask_node);
        redis_node->set_conn_errors(2019); // Trigger to refresh master nodes
        return HR_RETRY_UNCOND;
    }
    else
    {
        return HR_ERROR;
    }
}

void CRedisClient::fini()
{
    clear_all_master_nodes();
}

void CRedisClient::init()
{
    _enable_debug_log = true;
    _enable_info_log = true;
    _enable_error_log = true;

    try
    {
        const int num_nodes = parse_nodes(&_nodes, _raw_nodes_string);
        struct ErrorInfo errinfo;

        if (0 == num_nodes)
        {
            errinfo.errcode = ERROR_PARAMETER;
            errinfo.errmsg = format_string("[R3C_INIT][%s:%d] parameter[nodes] error: %s", __FILE__, __LINE__, _raw_nodes_string.c_str());
            errinfo.raw_errmsg = format_string("parameter[nodes] error: %s", _raw_nodes_string.c_str());
            if (_enable_error_log)
                (*g_error_log)("%s\n", errinfo.errmsg.c_str());
            THROW_REDIS_EXCEPTION(errinfo);
        }
        else if (1 == num_nodes)
        {
            if (!init_standlone(&errinfo))
                THROW_REDIS_EXCEPTION(errinfo);
        }
        else
        {
            if (!init_cluster(&errinfo))
                THROW_REDIS_EXCEPTION(errinfo);
        }
    }
    catch (...)
    {
        clear_all_master_nodes();
        throw;
    }
}

bool CRedisClient::init_standlone(struct ErrorInfo* errinfo)
{
    const Node& node = _nodes[0];
    _nodes_string = _raw_nodes_string;

    redisContext* redis_context = connect_redis_node(node, errinfo, false);
    if (NULL == redis_context)
    {
        return false;
    }
    else
    {
        CRedisMasterNode* redis_node = new CRedisMasterNode(std::string(""), node, redis_context);
        const std::pair<RedisMasterNodeTable::iterator, bool> ret =
                _redis_master_nodes.insert(std::make_pair(node, redis_node));
        R3C_ASSERT(ret.second);
        return true;
    }
}

bool CRedisClient::init_cluster(struct ErrorInfo* errinfo)
{
    const int num_nodes = static_cast<int>(_nodes.size());
    const uint64_t base = reinterpret_cast<uint64_t>(this);
    uint64_t seed = get_random_number(base);

    _slot2node.resize(CLUSTER_SLOTS);
    for (int i=0; i<num_nodes; ++i)
    {
        const int j = static_cast<int>(++seed % num_nodes);
        const Node& node = _nodes[j];
        std::vector<struct NodeInfo> nodes_info;

        redisContext* redis_context = connect_redis_node(node, errinfo, false);
        if (NULL == redis_context)
        {
            continue;
        }
        if (!list_cluster_nodes(&nodes_info, errinfo, redis_context, node))
        {
            redisFree(redis_context);
            continue;
        }
        else
        {
            std::vector<struct NodeInfo> replication_nodes_info;

            redisFree(redis_context);
            redis_context = NULL;
            if (init_master_nodes(nodes_info, &replication_nodes_info, errinfo))
            {
                if (_read_policy != RP_ONLY_MASTER)
                    init_replica_nodes(replication_nodes_info);
                break;
            }
        }
    }

    return !_redis_master_nodes.empty();
}

bool CRedisClient::init_master_nodes(
        const std::vector<struct NodeInfo>& nodes_info,
        std::vector<struct NodeInfo>* replication_nodes_info,
        struct ErrorInfo* errinfo)
{
    int connected = 0; // 成功连接的master个数

    if (nodes_info.size() > 1)
    {
        _nodes_string.clear();
    }
    else
    {
        _nodes_string = _raw_nodes_string;
    }
    for (std::vector<struct NodeInfo>::size_type i=0; i<nodes_info.size(); ++i) // Traversing all master nodes
    {
        const struct NodeInfo& nodeinfo = nodes_info[i];

        if (nodes_info.size() > 1)
        {
            update_nodes_string(nodeinfo);
        }
        if (nodeinfo.is_master() && !nodeinfo.is_fail())
        {
            update_slots(nodeinfo);
            if (add_master_node(nodeinfo, errinfo))
                ++connected;
        }
        else if (nodeinfo.is_replica() && !nodeinfo.is_fail())
        {
            replication_nodes_info->push_back(nodeinfo);
        }
    }

    // 至少要有一个能够连接上
    return connected > 0;
}

void CRedisClient::init_replica_nodes(const std::vector<struct NodeInfo>& replication_nodes_info)
{
    for (std::vector<struct NodeInfo>::size_type i=0; i<replication_nodes_info.size(); ++i)
    {
        const struct NodeInfo& nodeinfo = replication_nodes_info[i];
        const NodeId& master_nodeid = nodeinfo.master_id;
        const NodeId& replica_nodeid = nodeinfo.id;
        const Node& replica_node = nodeinfo.node;

        CRedisMasterNode* redis_master_node = get_redis_master_node(master_nodeid);
        if (redis_master_node != NULL)
        {
            struct ErrorInfo errinfo;
            redisContext* redis_context = connect_redis_node(replica_node, &errinfo, true);
            if (redis_context != NULL)
            {
                CRedisReplicaNode* redis_replica_node = new CRedisReplicaNode(replica_nodeid, replica_node, redis_context);
                redis_master_node->add_replica_node(redis_replica_node);
            }
        }
    }
}

void CRedisClient::update_slots(const struct NodeInfo& nodeinfo)
{
    for (SlotSegment::size_type i=0; i<nodeinfo.slots.size(); ++i)
    {
        const std::pair<int, int>& slot_segment = nodeinfo.slots[i];
        for (int slot=slot_segment.first; slot<=slot_segment.second; ++slot)
            _slot2node[slot] = nodeinfo.node;
    }
}

// 几种需要刷新master情况：
// 1) 遇到MOVED错误（可立即重刷）
// 2) master挂起（能够连接，但不能服务，立即重刷一般无效，得等主从切换后）
void CRedisClient::refresh_master_node_table(struct ErrorInfo* errinfo, const Node* error_node)
{
    const int num_nodes = static_cast<int>(_redis_master_nodes.size());
    uint64_t seed = reinterpret_cast<uint64_t>(this) - num_nodes;
    const int k = static_cast<int>(seed % num_nodes);
    RedisMasterNodeTable::iterator iter = _redis_master_nodes.begin();

    for (int i=0; i<k; ++i)
    {
        ++iter;
        if (iter == _redis_master_nodes.end())
            iter = _redis_master_nodes.begin();
    }
    for (int i=0; i<num_nodes; ++i)
    {
        const Node& node = iter->first;
        CRedisMasterNode* redis_node = iter->second;
        ++iter;

        // error_node可能已是一个有问题的节点，所以最好避开它
        if ((NULL==error_node) || (node!=*error_node))
        {
            redisContext* redis_context = redis_node->get_redis_context();

            if (NULL == redis_context)
            {
                redis_context = connect_redis_node(node, errinfo, false);
            }
            if (redis_context != NULL)
            {
                std::vector<struct NodeInfo> nodes_info;

                redis_node->set_redis_context(redis_context);
                if (list_cluster_nodes(&nodes_info, errinfo, redis_context, node))
                {
                    std::vector<struct NodeInfo> replication_nodes_info;
                    clear_and_update_master_nodes(nodes_info, &replication_nodes_info, errinfo);
                    if (_read_policy != RP_ONLY_MASTER)
                        init_replica_nodes(replication_nodes_info);
                    break; // Continue is not safe, because `clear_and_update_master_nodes` will modify _redis_master_nodes
                }
            }
        }
        if (iter == _redis_master_nodes.end())
        {
            iter = _redis_master_nodes.begin();
        }
    }
}

void CRedisClient::clear_and_update_master_nodes(
        const std::vector<struct NodeInfo>& nodes_info,
        std::vector<struct NodeInfo>* replication_nodes_info,
        struct ErrorInfo* errinfo)
{
    NodeInfoTable master_nodeinfo_table;
    std::string nodes_string;

    if (nodes_info.size() > 1)
    {
        _nodes_string.clear();
    }
    for (std::vector<struct NodeInfo>::size_type i=0; i<nodes_info.size(); ++i) // Traversing all master nodes
    {
        const struct NodeInfo& nodeinfo = nodes_info[i];

        if (nodes_info.size() > 1)
        {
            update_nodes_string(nodeinfo);
        }
        if (nodeinfo.is_master() && !nodeinfo.is_fail())
        {
            // 可能只是一个或多个slot从一个master迁到另一个master，
            // 简单的全量更新slot和node间的关系，
            // 如果一对master和replica同时异常，则_slot2node会出现空洞
            update_slots(nodeinfo);
            master_nodeinfo_table.insert(std::make_pair(nodeinfo.node, nodeinfo));

            if (_redis_master_nodes.count(nodeinfo.node) <= 0)
            {
                // New master
                add_master_node(nodeinfo, errinfo);
            }
        }
        else if (nodeinfo.is_replica() && !nodeinfo.is_fail())
        {
            replication_nodes_info->push_back(nodeinfo);
        }
    }

    clear_invalid_master_nodes(master_nodeinfo_table);
}

void CRedisClient::clear_invalid_master_nodes(const NodeInfoTable& master_nodeinfo_table)
{
    for (RedisMasterNodeTable::iterator node_iter=_redis_master_nodes.begin(); node_iter!=_redis_master_nodes.end();)
    {
        const Node& node = node_iter->first;
        const NodeInfoTable::const_iterator info_iter = master_nodeinfo_table.find(node);

        if (info_iter != master_nodeinfo_table.end())
        {
            // 保持不变，仍然是master
            ++node_iter;
        }
        else
        {
            CRedisMasterNode* master_node = node_iter->second;
            const NodeId nodeid = master_node->get_nodeid();
            if (_enable_info_log)
                (*g_info_log)("[R3C_CLEAR_INVALID][%s:%d] %s is removed because it is not a master now\n", __FILE__, __LINE__, master_node->str().c_str());

#if __cplusplus < 201103L
            _redis_master_nodes.erase(node_iter++);
#else
            node_iter = _redis_master_nodes.erase(node_iter);
#endif
            delete master_node;
            _redis_master_nodes_id.erase(nodeid);
        }
    }
}

bool CRedisClient::add_master_node(const NodeInfo& nodeinfo, struct ErrorInfo* errinfo)
{
    const NodeId& nodeid = nodeinfo.id;
    const Node& node = nodeinfo.node;
    redisContext* redis_context = connect_redis_node(node, errinfo, false);
    CRedisMasterNode* master_node = new CRedisMasterNode(nodeid, node, redis_context);

    const std::pair<RedisMasterNodeTable::iterator, bool> ret =
            _redis_master_nodes.insert(std::make_pair(node, master_node));
    R3C_ASSERT(ret.second);
    if (!ret.second)
        delete master_node;
    _redis_master_nodes_id[nodeid] = node;
    return redis_context != NULL;
}

void CRedisClient::clear_all_master_nodes()
{
    for (RedisMasterNodeTable::iterator iter=_redis_master_nodes.begin(); iter!=_redis_master_nodes.end(); ++iter)
    {
        CRedisMasterNode* master_node = iter->second;
        delete master_node;
    }
    _redis_master_nodes.clear();
    _redis_master_nodes_id.clear();
}

void CRedisClient::update_nodes_string(const NodeInfo& nodeinfo)
{
    std::string node_str;
    if (_nodes_string.empty())
        _nodes_string = node2string(nodeinfo.node, &node_str);
    else
        _nodes_string = _nodes_string + std::string(",") + node2string(nodeinfo.node, &node_str);
}

redisContext* CRedisClient::connect_redis_node(const Node& node, struct ErrorInfo* errinfo, bool readonly) const
{
    redisContext* redis_context = NULL;

    errinfo->clear();
    if (_enable_debug_log)
    {
        (*g_debug_log)("[R3C_CONN][%s:%d] To connect %s with timeout: %dms\n",
                __FILE__, __LINE__, node2string(node).c_str(), _connect_timeout_milliseconds);
    }
    if (_connect_timeout_milliseconds <= 0)
    {
        redis_context = redisConnect(node.first.c_str(), node.second);
    }
    else
    {
        struct timeval timeout;
        timeout.tv_sec = _connect_timeout_milliseconds / 1000;
        timeout.tv_usec = (_connect_timeout_milliseconds % 1000) * 1000;
        redis_context = redisConnectWithTimeout(node.first.c_str(), node.second, timeout);
    }

    if (NULL == redis_context)
    {
        // can't allocate redis context
        errinfo->errcode = ERROR_REDIS_CONTEXT;
        errinfo->raw_errmsg = "can not allocate redis context";
        errinfo->errmsg = format_string("[R3C_CONN][%s:%d][%s:%d] %s",
                __FILE__, __LINE__, node.first.c_str(), node.second, errinfo->raw_errmsg.c_str());
        if (_enable_error_log)
            (*g_error_log)("%s\n", errinfo->errmsg.c_str());
    }
    else if (redis_context->err != 0)
    {
        // #define REDIS_ERR_IO 1 /* Error in read or write */
        // redis_context->errstr
        //
        // Connection refused
        //
        // errno: EADDRNOTAVAIL(99)
        // err: REDIS_ERR_IO(1)
        // Cannot assign requested address

        errinfo->errcode = ERROR_INIT_REDIS_CONN;
        errinfo->raw_errmsg = redis_context->errstr;
        if (REDIS_ERR_IO == redis_context->err)
        {
            errinfo->errmsg = format_string("[R3C_CONN][%s:%d][%s:%d] (errno:%d,err:%d)%s",
                    __FILE__, __LINE__, node.first.c_str(), node.second,
                    errno, redis_context->err, errinfo->raw_errmsg.c_str());
        }
        else
        {
            errinfo->errmsg = format_string("[R3C_CONN][%s:%d][%s:%d] (err:%d)%s",
                    __FILE__, __LINE__, node.first.c_str(), node.second,
                    redis_context->err, errinfo->raw_errmsg.c_str());
        }
        if (_enable_error_log)
        {
            (*g_error_log)("%s\n", errinfo->errmsg.c_str());
        }
        redisFree(redis_context);
        redis_context = NULL;
    }
    else
    {
        if (_enable_debug_log)
        {
            (*g_debug_log)("[R3C_CONN][%s:%d] Connect %s successfully with readwrite timeout: %dms\n",
                    __FILE__, __LINE__, node2string(node).c_str(), _readwrite_timeout_milliseconds);
        }
        if (_readwrite_timeout_milliseconds > 0)
        {
            struct timeval data_timeout;
            data_timeout.tv_sec = _readwrite_timeout_milliseconds / 1000;
            data_timeout.tv_usec = (_readwrite_timeout_milliseconds % 1000) * 1000;

            if (REDIS_ERR == redisSetTimeout(redis_context, data_timeout))
            {
                // REDIS_ERR_IO == redis_context->err
                errinfo->errcode = ERROR_INIT_REDIS_CONN;
                errinfo->raw_errmsg = redis_context->errstr;
                errinfo->errmsg = format_string("[R3C_CONN][%s:%d][%s:%d] (errno:%d,err:%d)%s",
                        __FILE__, __LINE__, node.first.c_str(), node.second,
                        errno, redis_context->err, errinfo->raw_errmsg.c_str());
                if (_enable_error_log)
                    (*g_error_log)("%s\n", errinfo->errmsg.c_str());
                redisFree(redis_context);
                redis_context = NULL;
            }
        }
        if ((0 == errinfo->errcode) && !_password.empty())
        {
            const RedisReplyHelper redis_reply = (redisReply*)redisCommand(redis_context, "AUTH %s", _password.c_str());

            if (redis_reply && 0==strcmp(redis_reply->str, "OK"))
            {
                // AUTH success
                if (_enable_info_log)
                    (*g_info_log)("[R3C_AUTH][%s:%d] Connect redis://%s:%d success\n",
                            __FILE__, __LINE__, node.first.c_str(), node.second);
            }
            else
            {
                // AUTH failed
                if (!redis_reply)
                {
                    errinfo->errcode = ERROR_REDIS_AUTH;
                    errinfo->raw_errmsg = "authorization failed";
                }
                else
                {
                    extract_errtype(redis_reply.get(), &errinfo->errtype);
                    errinfo->errcode = ERROR_REDIS_AUTH;
                    errinfo->raw_errmsg = redis_reply->str;
                }

                errinfo->errmsg = format_string("[R3C_AUTH][%s:%d][%s:%d] %s",
                        __FILE__, __LINE__, node.first.c_str(), node.second, errinfo->raw_errmsg.c_str());
                if (_enable_error_log)
                    (*g_error_log)("%s\n", errinfo->errmsg.c_str());
                redisFree(redis_context);
                redis_context = NULL;
            }
        }
    }
    if (readonly && redis_context!=NULL)
    {
        const RedisReplyHelper redis_reply = (redisReply*)redisCommand(redis_context, "READONLY");

        if (redis_reply && 0==strcmp(redis_reply->str, "OK"))
        {
            // READONLY success
            if (_enable_info_log)
                (*g_debug_log)("[R3C_READONLY][%s:%d] READONLY redis://%s:%d success\n",
                        __FILE__, __LINE__, node.first.c_str(), node.second);
        }
        else
        {
            // READONLY failed
            if (!redis_reply)
            {
                errinfo->errcode = ERROR_REDIS_READONLY;
                errinfo->raw_errmsg = "readonly failed";
            }
            else
            {
                extract_errtype(redis_reply.get(), &errinfo->errtype);
                errinfo->errcode = ERROR_REDIS_READONLY;
                errinfo->raw_errmsg = redis_reply->str;
            }

            errinfo->errmsg = format_string("[R3C_READONLY][%s:%d][%s:%d] %s",
                    __FILE__, __LINE__, node.first.c_str(), node.second, errinfo->raw_errmsg.c_str());
            if (_enable_error_log)
                (*g_error_log)("%s\n", errinfo->errmsg.c_str());
            redisFree(redis_context);
            redis_context = NULL;
        }
    }

    return redis_context;
}

CRedisNode* CRedisClient::get_redis_node(
        int slot, bool readonly,
        const Node* ask_node, struct ErrorInfo* errinfo)
{
    CRedisNode* redis_node = NULL;
    redisContext* redis_context = NULL;

    do
    {
        if (-1 == slot)
        {
            // Standalone（单机redis）
            R3C_ASSERT(!_redis_master_nodes.empty());
            redis_node = _redis_master_nodes.begin()->second;
            redis_context = redis_node->get_redis_context();
            if (NULL == redis_context)
            {
                redis_context = connect_redis_node(redis_node->get_node(), errinfo, false);
                redis_node->set_redis_context(redis_context);
            }
            break;
        }
        else
        {
            // Cluster（集群redis）
            R3C_ASSERT(slot>=0 && slot<CLUSTER_SLOTS);

            // clear_invalid_master_nodes可能将整个_master_nodes清空了，比如当整个集群短暂不可用时
            if (_redis_master_nodes.empty())
            {
                const int num_nodes = parse_nodes(&_nodes, _nodes_string);

                R3C_ASSERT(num_nodes > 1);
                if (!init_cluster(errinfo))
                {
                    break;
                }
            }
            {
                const Node& node = (NULL==ask_node)? _slot2node[slot]: *ask_node;
                const RedisMasterNodeTable::const_iterator iter = _redis_master_nodes.find(node);
                if (iter != _redis_master_nodes.end())
                {
                    redis_node = iter->second;
                }
            }
        }

        if (NULL == redis_node)
        {
            // 遇到空的slot，随机选一个
            redis_node = random_redis_master_node();
        }
        if (redis_node != NULL)
        {
            redis_context = redis_node->get_redis_context();

            if (NULL == redis_context)
            {
                redis_context = connect_redis_node(redis_node->get_node(), errinfo, false);
                redis_node->set_redis_context(redis_context);
            }
            if (!readonly || RP_ONLY_MASTER==_read_policy)
            {
                break;
            }
            if (redis_context!=NULL && RP_PRIORITY_MASTER==_read_policy)
            {
                break;
            }

            CRedisMasterNode* redis_master_node = (CRedisMasterNode*)redis_node;
            redis_node = redis_master_node->choose_node(_read_policy);
            redis_context = redis_node->get_redis_context();
            if (NULL == redis_context)
            {
                redis_context = connect_redis_node(redis_node->get_node(), errinfo, false);
                redis_node->set_redis_context(redis_context);
            }
            if (NULL == redis_context)
            {
                redis_node = redis_master_node;
            }
        }
    } while(false);

    return redis_node;
}

CRedisMasterNode* CRedisClient::get_redis_master_node(const NodeId& nodeid) const
{
    CRedisMasterNode* redis_master_node = NULL;
    RedisMasterNodeIdTable::const_iterator nodeid_iter = _redis_master_nodes_id.find(nodeid);
    if (nodeid_iter != _redis_master_nodes_id.end())
    {
        const Node& node = nodeid_iter->second;
        RedisMasterNodeTable::const_iterator node_iter = _redis_master_nodes.find(node);
        if (node_iter != _redis_master_nodes.end())
            redis_master_node = node_iter->second;
    }
    return redis_master_node;
}

CRedisMasterNode* CRedisClient::random_redis_master_node() const
{
    if (_redis_master_nodes.empty())
    {
        return NULL;
    }
    else
    {
        const int num_nodes = static_cast<int>(_nodes.size());
        const uint64_t base = reinterpret_cast<uint64_t>(this);
        const uint64_t seed = get_random_number(base);
        const int K = static_cast<int>(seed % num_nodes);

        RedisMasterNodeTable::const_iterator iter = _redis_master_nodes.begin();
        for (int i=0; i<K; ++i)
        {
            ++iter;
            if (iter == _redis_master_nodes.end())
                iter = _redis_master_nodes.begin();
        }
        return iter->second;
    }
}

bool
CRedisClient::list_cluster_nodes(
        std::vector<struct NodeInfo>* nodes_info,
        struct ErrorInfo* errinfo,
        redisContext* redis_context,
        const Node& node)
{
    const RedisReplyHelper redis_reply = (redisReply*)redisCommand(redis_context, "CLUSTER NODES");

    errinfo->clear();
    if (!redis_reply)
    {
        const int sys_errcode = errno;
        const int redis_errcode = redis_context->err;
        errinfo->errcode = ERROR_COMMAND;
        if (redis_errcode != 0)
            errinfo->raw_errmsg = redis_context->errstr;
        else
            errinfo->raw_errmsg = "redisCommand failed";
        errinfo->errmsg = format_string("[R3C_LIST_NODES][%s:%d][NODE:%s] (sys:%d,redis:%d)%s",
                __FILE__, __LINE__, node2string(node).c_str(), sys_errcode, redis_errcode, errinfo->raw_errmsg.c_str());
        if (_enable_error_log)
            (*g_error_log)("%s\n", errinfo->errmsg.c_str());
        if (_enable_info_log)
            (*g_info_log)("[%s:%d] %s\n", __FILE__, __LINE__, _nodes_string.c_str());
    }
    else if (REDIS_REPLY_ERROR == redis_reply->type)
    {
        // ERR This instance has cluster support disabled
        errinfo->errcode = ERROR_COMMAND;
        errinfo->raw_errmsg = redis_reply->str;
        errinfo->errmsg = format_string("[R3C_LIST_NODES][%s:%d][NODE:%s] %s",
                __FILE__, __LINE__, node2string(node).c_str(), redis_reply->str);
        if (_enable_error_log)
            (*g_error_log)("%s\n", errinfo->errmsg.c_str());
    }
    else if (redis_reply->type != REDIS_REPLY_STRING)
    {
        // Unexpected reply type
        errinfo->errcode = ERROR_UNEXCEPTED_REPLY_TYPE;
        errinfo->raw_errmsg = redis_reply->str;
        errinfo->errmsg = format_string("[R3C_LIST_NODES][%s:%d][NODE:%s] (type:%d)%s",
                __FILE__, __LINE__, node2string(node).c_str(), redis_reply->type, redis_reply->str);
        if (_enable_error_log)
            (*g_error_log)("%s\n", errinfo->errmsg.c_str());
    }
    else
    {
        /*
         * <id> <ip:port> <flags> <master> <ping-sent> <pong-recv> <config-epoch> <link-state> <slot> <slot> ... <slot>
         *
         * flags: A list of comma separated flags: myself, master, slave, fail?, fail, handshake, noaddr, noflags
         * ping-sent: Milliseconds unix time at which the currently active ping was sent, or zero if there are no pending pings
         * pong-recv: Milliseconds unix time the last pong was received
         * link-state: The state of the link used for the node-to-node cluster bus. We use this link to communicate with the node. Can be connected or disconnected
         *
         * `redis_reply->str` example:
         * 56686c7baad565d4370b8f1f6518a67b6cedb210 10.225.168.52:6381 slave 150f77d1000003811fb3c38c3768526a0b25ec31 0 1464662426768 22 connected
         * 150f77d1000003811fb3c38c3768526a0b25ec31 10.225.168.51:6379 myself,master - 0 0 22 connected 3278-5687 11092-11958
         * 6a7709bc680f7b224d0d20bdf7dd14db1f013baf 10.212.2.71:6381 master - 0 1464662429775 24 connected 8795-10922 11959-13107
         *
         * e008649f6f8340a495fc860f7a9a8155f91fcb93 10.212.2.72:6379@11382 myself,master - 0 1547374698000 35 connected 5461-10922 14148 [14148->-ec19be9a50b5416999ac0305c744d9b6c957c18d]
         */
        std::vector<std::string> lines;
        const int num_lines = split(&lines, std::string(redis_reply->str), std::string("\n"));

        if (num_lines < 1)
        {
            errinfo->errcode = ERROR_REPLY_FORMAT;
            errinfo->raw_errmsg = "reply nothing";
            errinfo->errmsg = format_string("[R3C_LIST_NODES][%s:%d][NODE:%s][REPLY:%s] %s",
                    __FILE__, __LINE__, node2string(node).c_str(), redis_reply->str, "reply nothing");
            if (_enable_error_log)
                (*g_error_log)("%s\n", errinfo->errmsg.c_str());
        }
        for (int row=0; row<num_lines; ++row)
        {
            std::vector<std::string> tokens;
            const std::string& line = lines[row];
            const int num_tokens = split(&tokens, line, std::string(" "));

            if (0 == num_tokens)
            {
                // Over
                errinfo->clear();
                break;
            }
            else if (num_tokens < 8)
            {
                nodes_info->clear();
                errinfo->errcode = ERROR_REPLY_FORMAT;
                errinfo->raw_errmsg = "reply format error";
                errinfo->errmsg = format_string("[R3C_LIST_NODES][%s:%d][NODE:%s][LINE:%s] %s",
                        __FILE__, __LINE__, node2string(node).c_str(), line.c_str(), "reply format error");
                if (_enable_error_log)
                    (*g_error_log)("%s\n", errinfo->errmsg.c_str());
                break;
            }
            else
            {
                NodeInfo nodeinfo;
                nodeinfo.id = tokens[0];

                if (!parse_node_string(tokens[1], &nodeinfo.node.first, &nodeinfo.node.second))
                {
                    nodes_info->clear();
                    errinfo->errcode = ERROR_REPLY_FORMAT;
                    errinfo->raw_errmsg = "reply format error";
                    errinfo->errmsg = format_string("[R3C_LIST_NODES][%s:%d][NODE:%s][TOKEN:%s][LINE:%s] %s",
                            __FILE__, __LINE__, node2string(node).c_str(), tokens[1].c_str(), line.c_str(), "reply format error");
                    if (_enable_error_log)
                        (*g_error_log)("%s\n", errinfo->errmsg.c_str());
                    break;
                }
                else
                {
                    nodeinfo.flags = tokens[2];
                    nodeinfo.master_id = tokens[3];
                    nodeinfo.ping_sent = atoi(tokens[4].c_str());
                    nodeinfo.pong_recv = atoi(tokens[5].c_str());
                    nodeinfo.epoch = atoi(tokens[6].c_str());
                    nodeinfo.connected = (tokens[7] == "connected");

                    // 49cadd758538f821b922738fd000b5a16ef64fc7 127.0.0.1:1384@11384 master - 0 1546317629187 14 connected 10923-16383
                    // a27a1ce7f8c5c5f79a1d09227eb80b73919ec795 127.0.0.1:1383@11383 master,fail - 1546317518930 1546317515000 12 connected
                    if (nodeinfo.is_master() && !nodeinfo.is_fail())
                    {
                        for (int col=8; col<num_tokens; ++col)
                        {
                            const std::string& token = tokens[col];

                            // 排除掉正在迁移的：
                            // [14148->-ec19be9a50b5416999ac0305c744d9b6c957c18d]
                            if (token[0] != '[')
                            {
                                std::pair<int, int> slot;
                                parse_slot_string(token, &slot.first, &slot.second);
                                nodeinfo.slots.push_back(slot);
                            }
                        }
                    }

                    if (_enable_debug_log)
                        (*g_debug_log)("[R3C_LIST_NODES][%s:%d][NODE:%s] %s\n",
                                __FILE__, __LINE__, node2string(node).c_str(), nodeinfo.str().c_str());
                    nodes_info->push_back(nodeinfo);
                }
            }
        }
    }

    return !nodes_info->empty();
}

// Extract error type, such as ERR, MOVED, WRONGTYPE, ...
void CRedisClient::extract_errtype(const redisReply* redis_reply, std::string* errtype) const
{
    if (redis_reply->len > 2)
    {
        const char* space_pos = strchr(redis_reply->str, ' ');

        if (space_pos != NULL)
        {
            const size_t len = static_cast<size_t>(space_pos - redis_reply->str);

            if (len > 2)
            {
                if (isupper(redis_reply->str[0]) &&
                    isupper(redis_reply->str[1]) &&
                    isupper(redis_reply->str[2]))
                {
                    errtype->assign(redis_reply->str, len);
                }
            }
        }
    }
}

bool CRedisClient::get_value(const redisReply* redis_reply, std::string* value)
{
    value->clear();

    if (REDIS_REPLY_NIL == redis_reply->type)
    {
        return false;
    }
    else
    {
        if (redis_reply->len > 0)
            value->assign(redis_reply->str, redis_reply->len);
        else
            value->clear();
        return true;
    }
}

int CRedisClient::get_values(const redisReply* redis_reply, std::vector<std::string>* values)
{
    values->clear();

    if (redis_reply->elements > 0)
    {
        values->resize(redis_reply->elements);
        for (size_t i=0; i<redis_reply->elements; ++i)
        {
            const struct redisReply* value_reply = redis_reply->element[i];
            std::string& value = (*values)[i];

            if (value_reply->type != REDIS_REPLY_NIL)
            {
                value.assign(value_reply->str, value_reply->len);
            }
        }
    }
    return static_cast<int>(redis_reply->elements);
}

int CRedisClient::get_values(const redisReply* redis_reply, std::set<std::string>* values)
{
    values->clear();

    if (redis_reply->elements > 0)
    {
        for (size_t i=0; i<redis_reply->elements; ++i)
        {
            const struct redisReply* value_reply = redis_reply->element[i];

            if (value_reply->type != REDIS_REPLY_NIL)
            {
                const std::string v(value_reply->str, value_reply->len);
                values->insert(v);
            }
        }
    }
    return static_cast<int>(redis_reply->elements);
}

int CRedisClient::get_values(const redisReply* redis_reply, std::vector<std::pair<std::string, int64_t> >* vec, bool withscores)
{
    size_t steps;

    vec->clear();
    if (withscores)
    {
        steps = 2;
        vec->resize(redis_reply->elements/2);
    }
    else
    {
        steps = 1;
        vec->resize(redis_reply->elements);
    }

    for (size_t i=0; i<redis_reply->elements; i+=steps)
    {
        if (!withscores)
        {
            const struct redisReply* v_reply = redis_reply->element[i];
            (*vec)[i].first.assign(v_reply->str, v_reply->len);
            (*vec)[i].second = 0;
        }
        else
        {
            const int j = i / 2;
            const struct redisReply* k_reply = redis_reply->element[i];
            const struct redisReply* v_reply = redis_reply->element[i+1];
            const std::string k(k_reply->str, k_reply->len);
            const std::string v(v_reply->str, v_reply->len);
            (*vec)[j].first = k;
            (*vec)[j].second = static_cast<int64_t>(atoll(v.c_str()));
        }
    }

    return static_cast<int>(vec->size());
}

int CRedisClient::get_values(const redisReply* redis_reply, std::map<std::string, std::string>* map)
{
    map->clear();

    for (size_t i=0; i<redis_reply->elements; i+=2)
    {
        const struct redisReply* f_reply = redis_reply->element[i];
        const struct redisReply* v_reply = redis_reply->element[i+1];
        const std::string f(f_reply->str, f_reply->len);
        const std::string v(v_reply->str, v_reply->len);
        (*map)[f] = v;
    }

    return static_cast<int>(redis_reply->elements/2);
}

int
CRedisClient::get_values(
        const redisReply* redis_reply,
        const std::vector<std::string>& fields,
        bool keep_null,
        std::map<std::string, std::string>* map)
{
    map->clear();

    for (size_t i=0; i<redis_reply->elements; ++i)
    {
        const struct redisReply* value_reply = redis_reply->element[i];

        if (value_reply->type != REDIS_REPLY_NIL)
        {
            (*map)[fields[i]].assign(value_reply->str, value_reply->len);
        }
        else
        {
            if (keep_null)
            {
                (*map)[fields[i]] = "";
            }
        }
    }

    return static_cast<int>(map->size());
}

int CRedisClient::get_values(const redisReply* redis_reply, std::vector<int64_t>* values)
{
    if (NULL == values)
    {
        return -1;
    }
    if (redis_reply->elements > 0)
    {
        values->resize(redis_reply->elements, 0);

        for (size_t i=0; i<redis_reply->elements; ++i)
        {
            (*values)[i] = static_cast<int64_t>(redis_reply->element[i]->integer);
        }
    }

    return static_cast<int>(redis_reply->elements);
}

// Example:
//
// [0]XREADGROUP
//   [0:0]REPLY_ARRAY(2)
//     [1:0]REPLY_STRING: topic0
//     [1:1]REPLY_ARRAY(2)
//       [2:0]REPLY_ARRAY(2)
//         [3:0]REPLY_STRING: 1547510218547-0
//         [3:1]REPLY_ARRAY(6)
//           [4:0]REPLY_STRING: field00
//           [4:1]REPLY_STRING: value00
//           [4:2]REPLY_STRING: field01
//           [4:3]REPLY_STRING: value01
//           [4:4]REPLY_STRING: field02
//           [4:5]REPLY_STRING: value02
//       [2:1]REPLY_ARRAY(2)
//         [3:0]REPLY_STRING: 1547510237790-0
//         [3:1]REPLY_ARRAY(6)
//           [4:0]REPLY_STRING: field00
//           [4:1]REPLY_STRING: value00
//           [4:2]REPLY_STRING: field01
//           [4:3]REPLY_STRING: value01
//           [4:4]REPLY_STRING: field02
//           [4:5]REPLY_STRING: value02
//   [0:1]REPLY_ARRAY(2)
//     [1:0]REPLY_STRING: topic1
//     [1:1]REPLY_ARRAY(2)
//       [2:0]REPLY_ARRAY(2)
//         [3:0]REPLY_STRING: 1547510218547-0
//         [3:1]REPLY_ARRAY(6)
//           [4:0]REPLY_STRING: field10
//           [4:1]REPLY_STRING: value10
//           [4:2]REPLY_STRING: field11
//           [4:3]REPLY_STRING: value11
//           [4:4]REPLY_STRING: field12
//           [4:5]REPLY_STRING: value12
//       [2:1]REPLY_ARRAY(2)
//         [3:0]REPLY_STRING: 1547510237790-0
//         [3:1]REPLY_ARRAY(6)
//           [4:0]REPLY_STRING: field10
//           [4:1]REPLY_STRING: value10
//           [4:2]REPLY_STRING: field11
//           [4:3]REPLY_STRING: value11
//           [4:4]REPLY_STRING: field12
//           [4:5]REPLY_STRING: value12
int CRedisClient::get_values(const redisReply* redis_reply, std::vector<Stream>* values)
{
    R3C_ASSERT(REDIS_REPLY_NIL==redis_reply->type || REDIS_REPLY_ARRAY==redis_reply->type);

    if (NULL == values)
    {
        return -1;
    }
    if (REDIS_REPLY_NIL == redis_reply->type)
    {
        return 0;
    }
    else
    {
        const size_t num_keys = redis_reply->elements;
        std::vector<Stream>& streams = *values;

        streams.resize(num_keys);
        for (size_t i=0; i<num_keys; ++i) // Traversing all keys
        {
            const redisReply* key_redis_reply = redis_reply->element[i];
            const redisReply* keyname_redis_reply = key_redis_reply->element[0];
            const redisReply* entries_redis_reply = key_redis_reply->element[1];
            const size_t num_entries = entries_redis_reply->elements;
            Stream& stream = streams[i];

            stream.key.assign(keyname_redis_reply->str, keyname_redis_reply->len);
            stream.entries.resize(num_entries);
            for (size_t j=0; j<num_entries; ++j) // Traversing all entries
            {
                const redisReply* entry_redis_reply = entries_redis_reply->element[j]; // ID
                const redisReply* id_redis_reply = entry_redis_reply->element[0];
                const redisReply* fvpairs_redis_reply = entry_redis_reply->element[1];
                StreamEntry& entry = stream.entries[j];

                entry.id.assign(id_redis_reply->str, id_redis_reply->len);
                entry.fvpairs.resize(fvpairs_redis_reply->elements/2);
                for (size_t k=0,h=0; k<fvpairs_redis_reply->elements; k+=2,++h) // Traversing all field-value pairs
                {
                    const redisReply* field_redis_reply = fvpairs_redis_reply->element[k]; // Field
                    const redisReply* value_redis_reply = fvpairs_redis_reply->element[k+1]; // Value
                    FVPair& fvpair = entry.fvpairs[h];

                    fvpair.field.assign(field_redis_reply->str, field_redis_reply->len);
                    fvpair.value.assign(value_redis_reply->str, value_redis_reply->len);
                }
            }
        }

        return static_cast<int>(num_keys);
    }
}

// [0]XRANGE
//   [0:0]REPLY_ARRAY(2)
//     [1:0]REPLY_STRING: 1547557754696-0
//     [1:1]REPLY_ARRAY(6)
//       [2:0]REPLY_STRING: field00
//       [2:1]REPLY_STRING: value00
//       [2:2]REPLY_STRING: field01
//       [2:3]REPLY_STRING: value01
//       [2:4]REPLY_STRING: field02
//       [2:5]REPLY_STRING: value02
//   [0:1]REPLY_ARRAY(2)
//     [1:0]REPLY_STRING: 1547557754697-0
//     [1:1]REPLY_ARRAY(6)
//       [2:0]REPLY_STRING: field10
//       [2:1]REPLY_STRING: value10
//       [2:2]REPLY_STRING: field11
//       [2:3]REPLY_STRING: value11
//       [2:4]REPLY_STRING: field12
//       [2:5]REPLY_STRING: value12
int CRedisClient::get_values(const redisReply* redis_reply, std::vector<StreamEntry>* values)
{
    R3C_ASSERT(REDIS_REPLY_NIL==redis_reply->type || REDIS_REPLY_ARRAY==redis_reply->type);

    if (NULL == values)
    {
        return -1;
    }
    if (REDIS_REPLY_NIL == redis_reply->type)
    {
        return 0;
    }
    else
    {
        const size_t num_entries = redis_reply->elements;
        std::vector<StreamEntry>& values_ref = *values;

        values_ref.resize(num_entries);
        for (size_t j=0; j<num_entries; ++j) // Traversing all IDs
        {
            const redisReply* idname_redis_reply = redis_reply->element[j]->element[0];
            const redisReply* values_redis_reply = redis_reply->element[j]->element[1];
            StreamEntry& entry = values_ref[j];

            entry.id.assign(idname_redis_reply->str, idname_redis_reply->len);
            entry.fvpairs.resize(values_redis_reply->elements/2);
            for (size_t k=0,h=0; k<values_redis_reply->elements; k+=2,++h) // Traversing all values
            {
                const redisReply* field_redis_reply = values_redis_reply->element[k]; // Field
                const redisReply* value_redis_reply = values_redis_reply->element[k+1]; // Value
                FVPair& fvpair = entry.fvpairs[h];

                fvpair.field.assign(field_redis_reply->str, field_redis_reply->len);
                fvpair.value.assign(value_redis_reply->str, value_redis_reply->len);
            }
        }

        return static_cast<int>(num_entries);
    }
}

// [0]XPENDING
//   [0]REPLY_ARRAY(3)
//     [0:0]REPLY_ARRAY(4)
//         [1:0]REPLY_STRING: 1547876607592-0 // The ID of the message
//         [1:1]REPLY_STRING: c4 // The name of the consumer that fetched the message and has still to acknowledge it. We call it the current owner of the message..
//         [1:2]REPLY_INTEGER: 16473367 // The number of milliseconds that elapsed since the last time this message was delivered to this consumer.
//         [1:3]REPLY_INTEGER: 1 // The number of times this message was delivered
//     [0:1]REPLY_ARRAY(4)
//         [1:0]REPLY_STRING: 1547882190860-0
//         [1:1]REPLY_STRING: c4
//         [1:2]REPLY_INTEGER: 10891814
//         [1:3]REPLY_INTEGER: 1
//     [0:2]REPLY_ARRAY(4)
//         [1:0]REPLY_STRING: 1547882232235-0
//         [1:1]REPLY_STRING: c4
//         [1:2]REPLY_INTEGER: 10852378
//         [1:3]REPLY_INTEGER: 1
int CRedisClient::get_values(const redisReply* redis_reply, std::vector<struct DetailedPending>* pendings)
{
    const int num_pendings = static_cast<int>(redis_reply->elements);

    pendings->resize(num_pendings);
    for (int i=0; i<num_pendings; ++i)
    {
        const redisReply* entry_redis_reply = redis_reply->element[i];
        const redisReply* redis_reply0 = entry_redis_reply->element[0]; // The ID of the message
        const redisReply* redis_reply1 = entry_redis_reply->element[1]; // The name of the consumer
        const redisReply* redis_reply2 = entry_redis_reply->element[2]; // The number of milliseconds that elapsed since the last time this message was delivered to this consumer.
        const redisReply* redis_reply3 = entry_redis_reply->element[3]; // The number of times this message was delivered
        struct DetailedPending& pending = (*pendings)[i];

        pending.id.assign(redis_reply0->str, redis_reply0->len);
        pending.consumer.assign(redis_reply1->str, redis_reply1->len);
        pending.elapsed = static_cast<int64_t>(redis_reply2->integer);
        pending.delivered = static_cast<int64_t>(redis_reply3->integer);
    }

    return num_pendings;
}

// [0]XPENDING
//   [0]REPLY_ARRAY(4)
//     [0:0]REPLY_INTEGER: 4
//     [0:1]REPLY_STRING: 1547875178362-0
//     [0:2]REPLY_STRING: 1547876607592-0
//     [0:3]REPLY_ARRAY(4)
//         [1:0]REPLY_ARRAY(2)
//             [2:0]REPLY_STRING: c1
//             [2:1]REPLY_STRING: 1
//         [1:1]REPLY_ARRAY(2)
//             [2:0]REPLY_STRING: c2
//             [2:1]REPLY_STRING: 1
//         [1:2]REPLY_ARRAY(2)
//             [2:0]REPLY_STRING: c3
//             [2:1]REPLY_STRING: 1
//         [1:3]REPLY_ARRAY(2)
//             [2:0]REPLY_STRING: c4
//             [2:1]REPLY_STRING: 1
int CRedisClient::get_values(
        const redisReply* redis_reply,
        struct GroupPending* groups)
{
    const int num_consumers = static_cast<int>(redis_reply->elements);

    const redisReply* redis_reply0 = redis_reply->element[0]; // [0:0]REPLY_INTEGER: 4
    const redisReply* redis_reply1 = redis_reply->element[1]; // start
    const redisReply* redis_reply2 = redis_reply->element[2]; // end
    const redisReply* redis_reply3 = redis_reply->element[3];

    groups->start.assign(redis_reply1->str, redis_reply1->len);
    groups->end.assign(redis_reply2->str, redis_reply2->len);
    groups->consumers.resize(num_consumers);
    for (int i=0; i<num_consumers; ++i)
    {
        const redisReply* consumer_redis_reply = redis_reply3->element[i];
        const redisReply* consumer_name_redis_reply = consumer_redis_reply->element[0]; // consumer name
        const redisReply* consumer_number_redis_reply = consumer_redis_reply->element[1]; // number
        const std::string num_str(consumer_number_redis_reply->str, consumer_number_redis_reply->len);

        ConsumerPending& consumer = groups->consumers[i];
        consumer.name.assign(consumer_name_redis_reply->str, consumer_name_redis_reply->len);
        string2int(consumer_number_redis_reply->str, consumer_number_redis_reply->len, &consumer.count);
    }

    groups->count = static_cast<int>(redis_reply0->integer);
    return groups->count;
}

// Called by xinfo_consumers
int CRedisClient::get_values(const redisReply* redis_reply, std::vector<struct ConsumerInfo>* infos)
{
    const int num_consumers = static_cast<int>(redis_reply->elements);

    if (num_consumers > 0)
    {
        infos->resize(num_consumers);
        for (int i=0; i<num_consumers; ++i)
        {
            const redisReply* redis_reply0 = redis_reply->element[0];
            const redisReply* redis_reply1 = redis_reply->element[1];
            const redisReply* redis_reply2 = redis_reply->element[2];
            struct ConsumerInfo& info = (*infos)[i];
            info.name.assign(redis_reply0->str, redis_reply0->len);
            info.pendings = static_cast<int>(redis_reply1->integer);
            info.idletime = static_cast<int64_t>(redis_reply2->integer);
        }
    }
    return num_consumers;
}

// [0]XINFO
//   [0]REPLY_ARRAY(1)
//     [0:0]REPLY_ARRAY(8)
//         [1:0]REPLY_STRING: name
//         [1:1]REPLY_STRING: g1
//         [1:2]REPLY_STRING: consumers
//         [1:3]REPLY_INTEGER: 0
//         [1:4]REPLY_STRING: pending
//         [1:5]REPLY_INTEGER: 0
//         [1:6]REPLY_STRING: last-delivered-id
//         [1:7]REPLY_STRING: 1547905982459-0
//
// Called by xinfo_groups
int CRedisClient::get_values(const redisReply* redis_reply, std::vector<struct GroupInfo>* infos)
{
    const int num_groups = static_cast<int>(redis_reply->elements);

    if (num_groups > 0)
    {
        infos->resize(num_groups);
        for (int i=0; i<num_groups; ++i)
        {
            const redisReply* info_redis_reply = redis_reply->element[i];

            if (8 == info_redis_reply->elements)
            {
                (*infos)[i].name.assign(info_redis_reply->element[1]->str, info_redis_reply->element[1]->len); // group name
                (*infos)[i].consumers = static_cast<int>(info_redis_reply->element[3]->integer); // number of consumers
                (*infos)[i].consumers = static_cast<int>(info_redis_reply->element[5]->integer); // number of pendings
                (*infos)[i].name.assign(info_redis_reply->element[7]->str, info_redis_reply->element[7]->len); // last-delivered-id
            }
        }
    }
    return num_groups;
}

// [0]XINFO
//   [0]REPLY_ARRAY(14)
//     [0:0]REPLY_STRING: length
//     [0:1]REPLY_INTEGER: 1
//     [0:2]REPLY_STRING: radix-tree-keys
//     [0:3]REPLY_INTEGER: 1
//     [0:4]REPLY_STRING: radix-tree-nodes
//     [0:5]REPLY_INTEGER: 2
//     [0:6]REPLY_STRING: groups
//     [0:7]REPLY_INTEGER: 1
//     [0:8]REPLY_STRING: last-generated-id
//     [0:9]REPLY_STRING: 1547905982459-0
//     [0:10]REPLY_STRING: first-entry
//     [0:11]REPLY_ARRAY(2)
//         [1:0]REPLY_STRING: 1547905982459-0
//         [1:1]REPLY_ARRAY(10)
//             [2:0]REPLY_STRING: f0
//             [2:1]REPLY_STRING: v0
//             [2:2]REPLY_STRING: f1
//             [2:3]REPLY_STRING: v1
//             [2:4]REPLY_STRING: f2
//             [2:5]REPLY_STRING: v2
//             [2:6]REPLY_STRING: f3
//             [2:7]REPLY_STRING: v3
//             [2:8]REPLY_STRING: f4
//             [2:9]REPLY_STRING: v4
//     [0:12]REPLY_STRING: last-entry
//     [0:13]REPLY_ARRAY(2)
//         [1:0]REPLY_STRING: 1547905982459-0
//         [1:1]REPLY_ARRAY(10)
//             [2:0]REPLY_STRING: f0
//             [2:1]REPLY_STRING: v0
//             [2:2]REPLY_STRING: f1
//             [2:3]REPLY_STRING: v1
//             [2:4]REPLY_STRING: f2
//             [2:5]REPLY_STRING: v2
//             [2:6]REPLY_STRING: f3
//             [2:7]REPLY_STRING: v3
//             [2:8]REPLY_STRING: f4
//             [2:9]REPLY_STRING: v4
//
// Called by xinfo_stream
void CRedisClient::get_value(const redisReply* redis_reply, struct StreamInfo* info)
{
    //redis_reply->element[0]; // "length"
    info->entries = static_cast<int>(redis_reply->element[1]->integer); // value of length
    //redis_reply->element[2]; // "radix-tree-keys"
    info->radix_tree_keys = static_cast<int>(redis_reply->element[3]->integer); // value of "radix-tree-keys"
    //redis_reply->element[4]; // "radix-tree-nodes"
    info->radix_tree_nodes = static_cast<int>(redis_reply->element[5]->integer); // value of "radix-tree-nodes"
    //redis_reply->element[6]; // "groups"
    info->groups = static_cast<int>(redis_reply->element[7]->integer); // value of "groups"
    //redis_reply->element[8]; // "last-generated-id"
    info->last_generated_id.assign(redis_reply->element[9]->str, redis_reply->element[9]->len); // value of "last-generated-id"

    if (redis_reply->elements >= 14)
    {
        const redisReply* first_entry_redis_reply = redis_reply->element[11];
        const redisReply* last_entry_redis_reply = redis_reply->element[13];
        get_entry(first_entry_redis_reply, &info->first_entry);
        get_entry(last_entry_redis_reply, &info->last_entry);
    }
}

// [0:11]REPLY_ARRAY(2)
//     [1:0]REPLY_STRING: 1547905982459-0 // element[0]
//     [1:1]REPLY_ARRAY(10)               // element[1]
//         [2:0]REPLY_STRING: f0
//         [2:1]REPLY_STRING: v0
//         [2:2]REPLY_STRING: f1
//         [2:3]REPLY_STRING: v1
//         [2:4]REPLY_STRING: f2
//         [2:5]REPLY_STRING: v2
//         [2:6]REPLY_STRING: f3
//         [2:7]REPLY_STRING: v3
//         [2:8]REPLY_STRING: f4
//         [2:9]REPLY_STRING: v4
void CRedisClient::get_entry(const redisReply* entry_redis_reply, struct StreamEntry* entry)
{
    const redisReply* id_redis_reply = entry_redis_reply->element[0];
    const redisReply* fvpairs_redis_reply = entry_redis_reply->element[1];
    const int num_fvpairs = static_cast<int>(fvpairs_redis_reply->elements) / 2;

    entry->id.assign(id_redis_reply->str, id_redis_reply->len);
    entry->fvpairs.resize(num_fvpairs);
    for (size_t i=0; i<fvpairs_redis_reply->elements; i+=2)
    {
        const redisReply* field_redis_reply = fvpairs_redis_reply->element[i];
        const redisReply* value_redis_reply = fvpairs_redis_reply->element[i+1];
        const size_t j = i / 2;
        struct FVPair& fvpair = entry->fvpairs[j];

        fvpair.field.assign(field_redis_reply->str, field_redis_reply->len);
        fvpair.value.assign(value_redis_reply->str, value_redis_reply->len);
    }
}

} // namespace r3c {
