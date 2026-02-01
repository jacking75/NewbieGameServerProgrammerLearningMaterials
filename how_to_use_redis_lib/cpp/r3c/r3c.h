// Writed by yijian (eyjian@qq.com)
// R3C is a C++ client for redis based on hiredis (https://github.com/redis/hiredis)
#ifndef REDIS_CLUSTER_CLIENT_H
#define REDIS_CLUSTER_CLIENT_H

#ifdef _MSC_VER
#pragma comment(lib, "ws2_32.lib")
#pragma comment(lib, "hiredis.lib")

#pragma warning(push)
#pragma warning(disable:4200)
#include <hiredis.h>
#pragma warning(pop)

#include <winsock.h>
#else
#include <hiredis/hiredis.h>
#endif

#include <inttypes.h>
#include <stdint.h>
#include <map>
#include <set>
#include <string>
#include <vector>
#include <unordered_map>

#define R3C_VERSION 0x000020
#define R3C_MAJOR 0x00
#define R3C_MINOR 0x00
#define R3C_PATCH 0x20

namespace r3c {

extern int NUM_RETRIES /*=15*/; // The default number of retries is 15 (CLUSTERDOWN cost more than 6s)
extern int CONNECT_TIMEOUT_MILLISECONDS /*=2000*/; // Connection timeout in milliseconds
extern int READWRITE_TIMEOUT_MILLISECONDS /*=2000*/; // Receive and send timeout in milliseconds

enum ReadPolicy
{
    RP_ONLY_MASTER, // Always read from master
    RP_PRIORITY_MASTER,
    RP_PRIORITY_REPLICA,
    RP_READ_REPLICA
};

enum ZADDFLAG
{
    Z_NS, // Don't set options
    Z_XX, // Only update elements that already exist. Never add elements.
    Z_NX, // Don't update already existing elements. Always add new elements.
    Z_CH  // Modify the return value from the number of new elements added
};
extern std::string zaddflag2str(ZADDFLAG zaddflag);

////////////////////////////////////////////////////////////////////////////////

typedef std::pair<std::string, uint16_t> Node; // first is IP, second is port
typedef std::string NodeId;
typedef std::vector<std::pair<int, int> > SlotSegment; // first is begin slot, second is end slot

struct NodeHasher
{
    size_t operator ()(const Node& node) const;
};

extern std::string& node2string(const Node& node, std::string* str);
extern std::string node2string(const Node& node);

struct NodeInfo
{
    Node node;
    std::string id;         // The node ID, a 40 characters random string generated when a node is created and never changed again (unless CLUSTER RESET HARD is used)
    std::string flags;      // A list of comma separated flags: myself, master, slave, fail?, fail, handshake, noaddr, noflags
    std::string master_id;  // The replication master
    int ping_sent;          // Milliseconds unix time at which the currently active ping was sent, or zero if there are no pending pings
    int pong_recv;          // Milliseconds unix time the last pong was received
    int epoch;              // The configuration epoch (or version) of the current node (or of the current master if the node is a slave). Each time there is a failover, a new, unique, monotonically increasing configuration epoch is created. If multiple nodes claim to serve the same hash slots, the one with higher configuration epoch wins
    bool connected;         // The state of the link used for the node-to-node cluster bus
    SlotSegment slots;      // A hash slot number or range

    std::string str() const;
    bool is_master() const;
    bool is_replica() const;
    bool is_fail() const;
};

    typedef std::unordered_map<Node, NodeInfo, NodeHasher> NodeInfoTable;


extern std::ostream& operator <<(std::ostream& os, const struct NodeInfo& nodeinfo);

// The helper for freeing redisReply automatically
// DO NOT use RedisReplyHelper for any nested redisReply
class RedisReplyHelper
{
public:
    RedisReplyHelper();
    RedisReplyHelper(const redisReply* redis_reply);
    RedisReplyHelper(const RedisReplyHelper& other);
    ~RedisReplyHelper();
    operator bool() const;
    void free();
    const redisReply* get() const;
    const redisReply* detach() const;
    RedisReplyHelper& operator =(const redisReply* redis_reply);
    RedisReplyHelper& operator =(const RedisReplyHelper& other);
    const redisReply* operator ->() const;
    std::ostream& operator <<(std::ostream& os);

private:
    mutable const redisReply* _redis_reply;
};

extern bool is_ask_error(const std::string& errtype);
extern bool is_clusterdown_error(const std::string& errtype);
extern bool is_moved_error(const std::string& errtype);
extern bool is_noauth_error(const std::string& errtype);
extern bool is_noscript_error(const std::string& errtype);
extern bool is_wrongtype_error(const std::string& errtype);
extern bool is_busygroup_error(const std::string& errtype);
extern bool is_nogroup_error(const std::string& errtype);
extern bool is_crossslot_error(const std::string& errtype);

// NOTICE: not thread safe
// A redis client than support redis cluster
//
// Recommended multithread solution:
// Use `__thread` to the global instance of CRedisClient for thread level.
//
// EXAMPLE:
// static __thread r3c::CRedisClient* stg_redis_client = NULL; // Thread-level global variable
// r3c::CRedisClient* get_redis_client()
// {
//     if (NULL == stg_redis_client)
//     {
//         stg_redis_client = new r3c::CRedisClient(REDIS_CLUSTER_NODES);
//         //pthread_cleanup_push(release_redis_client, NULL);
//     }
//     return stg_redis_client;
// }
//
// By calling pthread_cleanup_push() to registger a callback to release sg_redis_client when thread exits.
// EXAMPLE:
// void release_redis_client(void*)
// {
//     delete sg_redis_client;
//     stg_redis_client = NULL;
// }

struct FVPair;
struct SlotInfo;
class CRedisNode;
class CRedisMasterNode;
class CRedisReplicaNode;
class CommandMonitor;

// Redis命令参数
class CommandArgs
{
public:
    CommandArgs();
    ~CommandArgs();
    void set_key(const std::string& key);
    void set_command(const std::string& command);

public:
    void add_arg(const std::string& arg);
    void add_arg(int32_t arg);
    void add_arg(uint32_t arg);
    void add_arg(int64_t arg);
    void add_args(const std::vector<std::string>& args);
    void add_args(const std::vector<std::pair<std::string, std::string> >& values);
    void add_args(const std::map<std::string, std::string>& map);
    void add_args(const std::map<std::string, int64_t>& map, bool reverse);
    void add_args(const std::vector<FVPair>& fvpairs);
    void final();

public:
    int get_argc() const;
    const char** get_argv() const;
    const size_t* get_argvlen() const;
    const std::string& get_command() const;
    const std::string& get_key() const;

private:
    std::string _key;
    std::string _command;

private:
    std::vector<std::string> _args;
    int _argc;
    char** _argv;
    size_t* _argvlen;
};

struct ErrorInfo
{
    std::string raw_errmsg;
    std::string errmsg;
    std::string errtype; // The type of error, such as: ERR, MOVED, WRONGTYPE, ...
    int errcode;

    ErrorInfo();
    ErrorInfo(const std::string& raw_errmsg_, const std::string& errmsg_, const std::string& errtype_, int errcode_);
    void clear();
};

class CRedisException: public std::exception
{
public:
    // key maybe a binary value
    CRedisException(
            const struct ErrorInfo& errinfo,
            const char* file, int line,
            const std::string& node_ip=std::string("-"), uint16_t node_port=0,
            const std::string& command=std::string(""), const std::string& key=std::string("")) throw ();
    virtual ~CRedisException() throw () {}
    virtual const char* what() const throw ();
    int errcode() const { return _errinfo.errcode; }
    std::string str() const throw ();

public:
    const char* file() const throw () { return _file.c_str(); }
    int line() const throw () { return _line; }
    const char* node_ip() const throw () { return _node_ip.c_str(); }
    uint16_t node_port() const throw () { return _node_port; }
    const std::string& command() const throw() { return _command; }
    const std::string& key() const throw() { return _key; }
    const std::string& errtype() const throw () { return _errinfo.errtype; }
    const std::string& raw_errmsg() const throw () { return _errinfo.raw_errmsg; }
    const ErrorInfo& get_errinfo() const throw() { return _errinfo; }

private:
    const ErrorInfo _errinfo;
    const std::string _file;
    const int _line;
    const std::string _node_ip;
    const uint16_t _node_port;

private:
    std::string _command;
    std::string _key;
};

// FVPair: field-value pair
struct FVPair
{
    std::string field;
    std::string value;
};

// Entry uniquely identified by a id
struct StreamEntry
{
    std::string id; // Stream entry ID (milliseconds-sequence)
    std::vector<struct FVPair> fvpairs; // field-value pairs
};

// Stream uniquely identified by a key
struct Stream
{
    std::string key;
    std::vector<struct StreamEntry> entries;
};

extern std::ostream& operator <<(std::ostream& os, const std::vector<struct Stream>& streams);
extern std::ostream& operator <<(std::ostream& os, const std::vector<struct StreamEntry>& entries);
// Returns the number of IDs
extern int extract_ids(const std::vector<struct StreamEntry>& entries, std::vector<std::string>* ids);

struct ConsumerPending
{
    std::string name; // Consumer name
    int count; // Number of pending messages consumer has
};

struct GroupPending
{
    int count; // The total number of pending messages for this consumer group
    std::string start; // The smallest ID among the pending messages
    std::string end; // The greatest ID among the pending messages
    std::vector<struct ConsumerPending> consumers; // All consumers in the group with at least one pending message
};

// detailed information for a message in the pending entries list
struct DetailedPending
{
    std::string id; // The ID of the message (milliseconds-sequence)
    std::string consumer; // The name of the consumer that fetched the message and has still to acknowledge it. We call it the current owner of the message..
    int64_t elapsed; // Number of milliseconds that elapsed since the last time this message was delivered to this consumer.
    int64_t delivered; // Number of times this message was delivered
};

struct ConsumerInfo
{
    std::string name; // Consumer name
    int pendings; // Number of pending messages for this specific consumer
    int64_t idletime; // The idle time in milliseconds
};

struct GroupInfo
{
    std::string name; // Group name
    std::string last_delivered_id;
    int consumers; // Number of consumers known in that group
    int pendings; // Number of pending messages (delivered but not yet acknowledged) in that group
};

struct StreamInfo
{
    int entries; // Number of entries inside this stream
    int radix_tree_keys;
    int radix_tree_nodes;
    int groups; // Number of consumer groups associated with the stream
    std::string last_generated_id; // The last generated ID that may not be the same as the last entry ID in case some entry was deleted
    struct StreamEntry first_entry;
    struct StreamEntry last_entry;
};
extern std::ostream& operator <<(std::ostream& os, const struct StreamInfo& streaminfo);

// NOTICE:
// 1) ALL keys and values can be binary except EVAL commands.
class CRedisClient
{
public:
    // raw_nodes_string - Redis cluster nodes separated by comma,
    //                    EXAMPLE: 127.0.0.1:6379,127.0.0.1:6380,127.0.0.2:6379,127.0.0.3:6379,
    //                    standalone mode if only one node, else cluster mode.
    //
    // Particularly same nodes are allowed for cluster mode:
    // const std::string nodes = "127.0.0.1:6379,127.0.0.1:6379";
    CRedisClient(
            const std::string& raw_nodes_string,
            int connect_timeout_milliseconds=CONNECT_TIMEOUT_MILLISECONDS,
            int readwrite_timeout_milliseconds=READWRITE_TIMEOUT_MILLISECONDS,
            const std::string& password=std::string(""),
            ReadPolicy read_policy=RP_ONLY_MASTER
            );
    CRedisClient(
            const std::string& raw_nodes_string,
            const std::string& password,
            int connect_timeout_milliseconds=CONNECT_TIMEOUT_MILLISECONDS,
            int readwrite_timeout_milliseconds=READWRITE_TIMEOUT_MILLISECONDS,
            ReadPolicy read_policy=RP_ONLY_MASTER
            );
    CRedisClient(
            const std::string& raw_nodes_string,
            ReadPolicy read_policy,
            const std::string& password=std::string(""),
            int connect_timeout_milliseconds=CONNECT_TIMEOUT_MILLISECONDS,
            int readwrite_timeout_milliseconds=READWRITE_TIMEOUT_MILLISECONDS
            );
    ~CRedisClient();
    const std::string& get_raw_nodes_string() const;
    const std::string& get_nodes_string() const;
    std::string str() const;

    // Returns true if parameter nodes of ctor is composed of two or more nodes,
    // or false when only a node for standlone mode.
    bool cluster_mode() const;
    const char* get_mode_str() const;

public: // Control logs
    void enable_debug_log();
    void disable_debug_log();
    void enable_info_log();
    void disable_info_log();
    void enable_error_log();
    void disable_error_log();

public:
    int list_nodes(std::vector<struct NodeInfo>* nodes_info);

    // NOT SUPPORT CLUSTER
    //
    // Remove all keys from all databases.
    //
    // The time-complexity for this operation is O(N), N being the number of keys in all existing databases.
    void flushall();

    // NOT SUPPORT cluster mode
    void multi(const std::string& key=std::string(""), Node* which=NULL);

    // NOT SUPPORT cluster mode
    const RedisReplyHelper exec(const std::string& key=std::string(""), Node* which=NULL);

public: // KV
    // Set a key's time to live in seconds.
    // Time complexity: O(1)
    //
    // Returns true if the timeout was set, or false when key does not exist.
    bool expire(const std::string& key, uint32_t seconds, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Determine if a key exists.
    // Time complexity: O(1)
    // Returns true if the key exists, or false when the key does not exist.
    bool exists(const std::string& key, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Time complexity:
    // O(N) where N is the number of keys that will be removed.
    // When a key to remove holds a value other than a string,
    // the individual complexity for this key is O(M) where M is the number of elements in the list, set, sorted set or hash.
    // Removing a single key that holds a string value is O(1).
    //
    // Returns true, or false when key does not exist.
    bool del(const std::string& key, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Get the value of a key
    // Time complexity: O(1)
    // Returns false if key does not exist.
    bool get(const std::string& key, std::string* value, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Set the string value of a key.
    // Time complexity: O(1)
    void set(const std::string& key, const std::string& value, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Set the value of a key, only if the does not exist.
    // Time complexity: O(1)
    // Returns true if the key was set, or false the key was not set.
    bool setnx(const std::string& key, const std::string& value, Node* which=NULL, int num_retries=0);

    // Set the value and expiration of a key.
    // Time complexity: O(1)
    void setex(const std::string& key, const std::string& value, uint32_t expired_seconds, Node* which=NULL, int num_retries=NUM_RETRIES);
    bool setnxex(const std::string& key, const std::string& value, uint32_t expired_seconds, Node* which=NULL, int num_retries=0);

    // Get the values of all the given keys.
    //
    // Time complexity:
    // O(N) where N is the number of keys to retrieve.
    //
    // For every key that does not hold a string value or does not exist, the value will be empty string value.
    //
    // Returns the number of values.
    int mget(const std::vector<std::string>& keys, std::vector<std::string>* values, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Set multiple keys to multiple values.
    //
    // In cluster mode, mset guaranteed atomicity, or may partially success.
    //
    // Time complexity:
    // O(N) where N is the number of keys to set.
    int mset(const std::map<std::string, std::string>& kv_map, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Increment the integer value of a key by the given value.
    // Time complexity: O(1)
    // Returns the value of key after the increment.
    int64_t incrby(const std::string& key, int64_t increment, Node* which=NULL, int num_retries=0);

    // Atomically increment and expire a key with given seconds.
    // Expiration is set only if the value returned by incrby is equal to expired_increment.
    //
    // Based on EVAL, NOT SUPPORT binary key
    //
    // e.g.,
    // incrby(key, 1, 1, 10);
    int64_t incrby(const std::string& key, int64_t increment, int64_t expired_increment, uint32_t expired_seconds, Node* which=NULL, int num_retries=0);

    // Same as incrby(key, increment, increment, expired_seconds, which, num_retries)
    int64_t incrby(const std::string& key, int64_t increment, uint32_t expired_seconds, Node* which=NULL, int num_retries=0);

    // Determine the type stored at key.
    // Time complexity: O(1)
    bool key_type(const std::string& key, std::string* key_type, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Get the time to live for a key
    //
    // Time complexity: O(1)
    // Returns the remaining time to live of a key that has a timeout in seconds.
    // Returns -2 if the key does not exist.
    // Returns -1 if the key exists but has no associated expire.
    int64_t ttl(const std::string& key, Node* which=NULL, int num_retries=NUM_RETRIES);

    // NOT SUPPORTED CLUSTER
    //
    // Incrementally iterate sorted sets elements and associated scores.
    //
    // Time complexity:
    // O(1) for every call. O(N) for a complete iteration,
    // including enough command calls for the cursor to return back to 0. N is the number of elements inside the collection.
    //
    // Returns an updated cursor that the user needs to use as the cursor argument in the next call.
    int64_t scan(int64_t cursor, std::vector<std::string>* values, Node* which=NULL, int num_retries=NUM_RETRIES);
    int64_t scan(int64_t cursor, int count, std::vector<std::string>* values, Node* which=NULL, int num_retries=NUM_RETRIES);
    int64_t scan(int64_t cursor, const std::string& pattern, std::vector<std::string>* values, Node* which=NULL, int num_retries=NUM_RETRIES);

    // If pattern is empty, then not using MATCH,
    // If count is 0, then not using COUNT
    int64_t scan(int64_t cursor, const std::string& pattern, int count, std::vector<std::string>* values, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Execute a Lua script server side.
    //
    // NOTICE1: Based on EVAL, NOT SUPPORT binary key & value
    // NOTICE2: Key can not include newline character ('\n')
    //
    // Time complexity: Depends on the script that is executed.
    const RedisReplyHelper eval(const std::string& key, const std::string& lua_scripts, std::pair<std::string, uint16_t>* which=NULL, int num_retries=NUM_RETRIES);
    const RedisReplyHelper eval(const std::string& key, const std::string& lua_scripts, const std::vector<std::string>& parameters, Node* which=NULL, int num_retries=NUM_RETRIES);
    const RedisReplyHelper evalsha(const std::string& key, const std::string& sha1, const std::vector<std::string>& parameters, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Only standlone
    //
    // Example:
    //
    // const std::string lua_scripts = "for i=1,#ARGV,2 do redis.call('SET',KEYS[i],ARGV[i]); end;redis.status_reply('OK');";
    // std::vector<std::string> keys(3);
    // std::vector<std::string> parameters(3);
    // keys[0] = "K1";
    // keys[1] = "K2";
    // keys[2] = "K3";
    // parameters[0] = "1";
    // parameters[1] = "2";
    // parameters[2] = "3";
    //
    // eval(false, lua_scripts, keys, parameters);
    const RedisReplyHelper eval(const std::string& lua_scripts, const std::vector<std::string>& keys, const std::vector<std::string>& parameters, Node* which=NULL, int num_retries=NUM_RETRIES);
    const RedisReplyHelper evalsha(const std::string& sha1, const std::vector<std::string>& keys, const std::vector<std::string>& parameters, Node* which=NULL, int num_retries=NUM_RETRIES);

public: // HASH
    // Delete a hash field.
    // Time complexity: O(1)
    bool hdel(const std::string& key, const std::string& field, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Delete one or more hash fields.
    // Time complexity: O(N) where N is the number of fields to be removed.
    // Returns the number of fields that were removed from the hash, not including specified but non existing fields.
    int hdel(const std::string& key, const std::vector<std::string>& fields, Node* which=NULL, int num_retries=NUM_RETRIES);
    int hmdel(const std::string& key, const std::vector<std::string>& fields, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Determinte if a hash field exists.
    // Time complexity: O(1)
    // Returns true if the hash contains field, or false when the hash does not contain field, or key does not exist.
    bool hexists(const std::string& key, const std::string& field, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Get the number of fields in a hash
    // Time complexity: O(1)
    // Returns number of fields in the hash, or 0 when key does not exist.
    int hlen(const std::string& key, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Set the string value of a hash field.
    // Time complexity: O(1)
    // Returns true if field is a new field in the hash and value was set, or false.
    bool hset(const std::string& key, const std::string& field, const std::string& value, Node* which=NULL, int num_retries=NUM_RETRIES);
    bool hsetex(const std::string& key, const std::string& field, const std::string& value, uint32_t expired_seconds, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Set the value of a hash field, only if the field does not exists.
    //
    // Time complexity: O(1)
    // Returns true if field is a new field in the hash and value was set,
    // or field already exists in the hash and no operation was performed.
    bool hsetnx(const std::string& key, const std::string& field, const std::string& value, Node* which=NULL, int num_retries=0);

    // Based on EVAL, NOT SUPPORT binary key & field & value
    bool hsetnxex(const std::string& key, const std::string& field, const std::string& value, uint32_t expired_seconds, Node* which=NULL, int num_retries=0);

    // Time complexity: O(1)
    // Returns true if exists, or false when field is not present in the hash or key does not exist.
    bool hget(const std::string& key, const std::string& field, std::string* value, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Increment the integer value of a hash field by the given number.
    //
    // Time complexity: O(1)
    // Returns the value at field after the increment operation.
    int64_t hincrby(const std::string& key, const std::string& field, int64_t increment, Node* which=NULL, int num_retries=0);

    // Safely retry within the specified time by expired_seconds
    // uid Unique ID
    // Hash tag: {key}uid
    // Based on EVAL, NOT SUPPORT binary key & field
    bool hincrby(const std::string& key, const std::string& field, int64_t increment, const std::string& uid, uint32_t expired_seconds=60, int64_t* newvalue=NULL, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Based on EVAL, NOT SUPPORT binary key & field
    void hincrby(const std::string& key, const std::vector<std::pair<std::string, int64_t> >& increments, std::vector<int64_t>* newvalues=NULL, Node* which=NULL, int num_retries=0);

    // Based on EVAL, NOT SUPPORT binary key & field
    void hmincrby(const std::string& key, const std::vector<std::pair<std::string, int64_t> >& increments, std::vector<int64_t>* newvalues=NULL, Node* which=NULL, int num_retries=0);

    // Safely retry within the specified time by expired_seconds
    // uid Unique ID
    // Hash tag: {key}uid
    // Based on EVAL, NOT SUPPORT binary key & field
    bool hmincrby(const std::string& key, const std::vector<std::pair<std::string, int64_t> >& increments, const std::string& uid, uint32_t expired_seconds=60, std::vector<int64_t>* newvalues=NULL, Node* which=NULL, int num_retries=0);

    // Set multiple hash fields to multiple values.
    // Time complexity: O(N) where N is the number of fields being set.
    void hset(const std::string& key, const std::map<std::string, std::string>& map, Node* which=NULL, int num_retries=NUM_RETRIES);
    void hmset(const std::string& key, const std::map<std::string, std::string>& map, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Get the values of all the given hash fields.
    // Time complexity: O(N) where N is the number of fields being requested.
    int hget(const std::string& key, const std::vector<std::string>& fields, std::map<std::string, std::string>* map, bool keep_null=false, Node* which=NULL, int num_retries=NUM_RETRIES);
    int hmget(const std::string& key, const std::vector<std::string>& fields, std::map<std::string, std::string>* map, bool keep_null=false, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Get all the fields and values in a hash.
    // Time complexity: O(N) where N is the size of the hash.
    int hgetall(const std::string& key, std::map<std::string, std::string>* map, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Time complexity: O(1)
    //
    // Returns the string length of the value associated with field,
    // or zero when field is not present in the hash or key does not exist at all.
    int hstrlen(const std::string& key, const std::string& field, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Get all the fields in a hash.
    // Time complexity: O(N) where N is the size of the hash.
    int hkeys(const std::string& key, std::vector<std::string>* fields, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Get all the values in a hash.
    // Time complexity: O(N) where N is the size of the hash.
    int hvals(const std::string& key, std::vector<std::string>* vals, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Incrementally iterate hash fields and associated values.
    //
    // Time complexity:
    // O(1) for every call. O(N) for a complete iteration,
    // including enough command calls for the cursor to return back to 0. N is the number of elements inside the collection..
    //
    // Returns an updated cursor that the user needs to use as the cursor argument in the next call.
    int64_t hscan(const std::string& key, int64_t cursor,
            std::map<std::string, std::string>* map,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    int64_t hscan(const std::string& key, int64_t cursor, int count,
            std::map<std::string, std::string>* map,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    int64_t hscan(const std::string& key, int64_t cursor, const std::string& pattern,
            std::map<std::string, std::string>* map,
            Node* which=NULL, int num_retries=NUM_RETRIES);

    // If pattern is empty, then not using MATCH,
    // If count is 0, then not using COUNT
    int64_t hscan(const std::string& key, int64_t cursor, const std::string& pattern, int count,
            std::map<std::string, std::string>* map,
            Node* which=NULL, int num_retries=NUM_RETRIES);   
    
public: // LIST
    // Get the length of a list
    // Time complexity: O(1)
    int llen(const std::string& key, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Remove and get the first element in a list.
    // Time complexity: O(1)
    bool lpop(const std::string& key, std::string* value, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Prepend a value to a list.
    // Time complexity: O(1)
    int lpush(const std::string& key, const std::string& value, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Prepend one or multiple values to a list.
    // Time complexity: O(1)
    int lpush(const std::string& key, const std::vector<std::string>& values, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Inserts value at the head of the list stored at key, only if key already exists and holds a list.
    // Time complexity: O(1)
    // Returns the length of the list after the push operation.
    int lpushx(const std::string& key, const std::string& value, Node* which=NULL, int num_retries=0);

    // Get a range of elements from a list.
    //
    // Time complexity:
    // O(S+N) where S is the distance of start offset from HEAD for small lists,
    // from nearest end (HEAD or TAIL) for large lists;
    // and N is the number of elements in the specified range.
    //
    // Returns the number of elements in the specified range.
    int lrange(const std::string& key, int64_t start, int64_t end, std::vector<std::string>* values, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Trim a list to the specified range.
    // Time complexity:
    // O(N) where N is the number of elements to be removed by the operation.
    void ltrim(const std::string& key, int64_t start, int64_t end, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Sets the list element at index to value.
    // Time complexity:
    // O(N) where N is the length of the list. Setting either the first or the last element of the list is O(1).
    void lset(const std::string& key, int index, const std::string& value, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Inserts value in the list stored at key either before or after the reference value pivot.
    // Returns the length of the list after the insert operation,
    // or -1 when the value pivot was not found.
    int linsert(const std::string& key, const std::string& pivot, const std::string& value, bool before, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Removes the first count occurrences of elements equal to value from the list stored at key.
    // Time complexity: O(N) where N is the length of the list.
    //
    // The count argument influences the operation in the following ways:
    // count > 0: Remove elements equal to value moving from head to tail.
    // count < 0: Remove elements equal to value moving from tail to head.
    // count = 0: Remove all elements equal to value.
    //
    // Returns the number of removed elements.
    // Note that non-existing keys are treated like empty lists, so when key does
    // not exist, the command will always return 0.
    int lrem(const std::string& key, int count, const std::string& value, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Get the element at index index in the list stored at key.
    // The index is zero-based, so 0 means the first element, 1 the second element and so on.
    // Negative indices can be used to designate elements starting at the tail of the list.
    // Here, -1 means the last element, -2 means the penultimate and so forth.
    //
    // value the element at index index in the list stored at key.
    // Returns true if the element exists, or returns false.
    bool lindex(const std::string& key, int index, std::string* value, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Remove and get the last element in a list.
    // Time complexity: O(1)
    bool rpop(const std::string& key, std::string* value, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Append a value to a list.
    // Time complexity: O(1)
    // Returns the length of the list after the push operation.
    int rpush(const std::string& key, const std::string& value, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Append one or multiple values to a list.
    // Time complexity: O(1)
    // Returns the length of the list after the push operation.
    int rpush(const std::string& key, const std::vector<std::string>& values, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Inserts value at the tail of the list stored at key,
    // only if key already exists and holds a list. In contrary to RPUSH,
    // no operation will be performed when key does not yet exist.
    //
    // Time complexity: O(1)
    //
    // Returns the length of the list after the push operation.
    int rpushx(const std::string& key, const std::string& value, Node* which=NULL, int num_retries=0);

public: // SET
    // Returns the number of elements that were added to the set,
    // not including all the elements already present into the set.
    int sadd(const std::string& key, const std::string& value, Node* which=NULL, int num_retries=NUM_RETRIES);
    int sadd(const std::string& key, const std::vector<std::string>& values, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Returns the cardinality (number of elements) of the set, or 0 if key does not exist.
    int scard(const std::string& key, Node* which=NULL, int num_retries=NUM_RETRIES);
    bool sismember(const std::string& key, const std::string& value, Node* which=NULL, int num_retries=NUM_RETRIES);
    int smembers(const std::string& key, std::vector<std::string>* values, Node* which=NULL, int num_retries=NUM_RETRIES);
    int smembers(const std::string& key, std::set<std::string>* values, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Removes and returns a random elements from the set value store at key.
    // Time complexity: O(1)
    // Returns true if key exists, or false when key does not exist.
    bool spop(const std::string& key, std::string* value, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Removes and returns one or more random elements from the set value store at key.
    // Time complexity: O(1)
    // Returns the number of removed elements.
    int spop(const std::string& key, int count, std::vector<std::string>* values, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Returns true if key exists, or false when key does not exist.
    bool srandmember(const std::string& key, std::string* value, Node* which=NULL, int num_retries=NUM_RETRIES);
    // Returns number of values, or 0 when key does not exist.
    int srandmember(const std::string& key, int count, std::vector<std::string>* values, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Remove a member from a set.
    // Time complexity: O(1)
    // Returns the number of members that were removed from the set, not including non existing members.
    int srem(const std::string& key, const std::string& value, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Remove one or more members from a set.
    // Time complexity: O(N) where N is the number of members to be removed.
    // Returns the number of members that were removed from the set, not including non existing members.
    int srem(const std::string& key, const std::vector<std::string>& values, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Incrementally iterate set elements.
    //
    // Time complexity:
    // O(1) for every call. O(N) for a complete iteration,
    // including enough command calls for the cursor to return back to 0. N is the number of elements inside the collection..
    //
    // Returns an updated cursor that the user needs to use as the cursor argument in the next call.
    int64_t sscan(const std::string& key, int64_t cursor,
            std::vector<std::string>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    int64_t sscan(const std::string& key, int64_t cursor, int count,
            std::vector<std::string>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    int64_t sscan(const std::string& key, int64_t cursor, const std::string& pattern,
            std::vector<std::string>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);

    // Copies all members of source keys to destinationkey.
    // Time complexity: O(N) where N is the total number of elements in all given sets.
    // Returns the number of members that were in resulting set.
    int sunionstore(const std::string& destinationkey, const std::vector<std::string>& keys, Node* which=NULL, int num_retries=NUM_RETRIES);

    // If pattern is empty, then not using MATCH,
    // If count is 0, then not using COUNT
    int64_t sscan(const std::string& key, int64_t cursor, const std::string& pattern, int count,
            std::vector<std::string>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    int64_t sscan(const std::string& key, int64_t cursor, const std::string& pattern, int count,
            std::set<std::string>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);

public: // ZSET
    // Removes the specified members from the sorted set stored at key. Non existing members are ignored.
    //
    // Time complexity:
    // O(M*log(N)) with N being the number of elements in the sorted set and M the number of elements to be removed.
    //
    // Returns the number of members removed from the sorted set, not including non existing members.
    int zrem(const std::string& key, const std::string& field, Node* which=NULL, int num_retries=NUM_RETRIES);
    int zrem(const std::string& key, const std::vector<std::string>& fields, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Adds all the specified members with the specified scores to the sorted set stored at key.
    // If key does not exist, a new sorted set with the specified members as sole members is created,
    // like if the sorted set was empty.
    //
    // Time complexity:
    // O(log(N)) for each item added, where N is the number of elements in the sorted set.
    //
    // Returns The number of elements added to the sorted sets,
    // not including elements already existing for which the score was updated.
    int zadd(const std::string& key, const std::string& field, int64_t score, ZADDFLAG flag=Z_NS, Node* which=NULL, int num_retries=NUM_RETRIES);
    int zadd(const std::string& key, const std::map<std::string, int64_t>& map, ZADDFLAG flag=Z_NS, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Get the number of members in a sorted set.
    // Time complexity: O(1)
    // Returns the sorted set cardinality (number of elements) of the sorted set stored at key.
    int64_t zcard(const std::string& key, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Count the members in a sorted set with scores within the given values.
    //
    // Time complexity:
    // O(log(N)) with N being the number of elements in the sorted set.
    // Returns the number of elements in the sorted set at key with a score between min and max.
    int64_t zcount(const std::string& key, int64_t min, int64_t max , Node* which=NULL, int num_retries=NUM_RETRIES);

    // Increment the score of a member in a sorted set.
    //
    // Time complexity:
    // O(log(N)) where N is the number of elements in the sorted set.
    int64_t zincrby(const std::string& key, const std::string& field, int64_t increment, Node* which=NULL, int num_retries=0);

    // Return a range of members in a sorted set by index.
    //
    // Time complexity:
    // O(log(N)+M) with N being the number of elements in the sorted set and M the number of elements returned.
    int zrange(const std::string& key, int64_t start, int64_t end, bool withscores, std::vector<std::pair<std::string, int64_t> >* vec, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Return a range of members in a sorted set by index, with scores ordered from high to low.
    //
    // Time complexity:
    // O(log(N)+M) with N being the number of elements in the sorted set and M the number of elements returned.
    int zrevrange(const std::string& key, int64_t start, int64_t end, bool withscores, std::vector<std::pair<std::string, int64_t> >* vec, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Get all the elements in the sorted set at key with a score between min and max
    // (including elements with score equal to min or max).
    // The elements are considered to be ordered from low to high scores.
    //
    // The elements having the same score are returned in lexicographical order
    // (this follows from a property of the sorted set implementation in Redis and does not involve further computation).
    //
    // Time complexity:
    // O(log(N)+M) with N being the number of elements in the sorted set and M the number of elements being returned.
    // If M is constant (e.g. always asking for the first 10 elements with LIMIT), you can consider it O(log(N)).
    //
    // Return the number of elements with a score between min and max (including elements with score equal to min or max).
    int zrangebyscore(const std::string& key, int64_t min, int64_t max, bool withscores, std::vector<std::pair<std::string, int64_t> >* vec, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Get all the elements in the sorted set at key with a score between max and min
    // (including elements with score equal to max or min).
    // In contrary to the default ordering of sorted sets,
    // for this command the elements are considered to be ordered from high to low scores.
    //
    // The elements having the same score are returned in reverse lexicographical order.
    //
    // Time complexity:
    // O(log(N)+M) with N being the number of elements in the sorted set and M the number of elements being returned.
    // If M is constant (e.g. always asking for the first 10 elements with LIMIT), you can consider it O(log(N)).
    //
    // Return the number of elements with a score between max and min (including elements with score equal to max or min).
    int zrevrangebyscore(const std::string& key,
            int64_t max, int64_t min, bool withscores,
            std::vector<std::pair<std::string, int64_t> >* vec,
            Node* which=NULL, int num_retries=NUM_RETRIES);

    // Return a range of members in a sorted set by score with scores ordered from low to high.
    int zrangebyscore(const std::string& key,
            int64_t min, int64_t max, int64_t offset, int64_t count, bool withscores,
            std::vector<std::pair<std::string, int64_t> >* vec,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    // Return a range of members in a sorted set by score with scores ordered from high to low.
    int zrevrangebyscore(const std::string& key,
            int64_t max, int64_t min, int64_t offset, int64_t count, bool withscores,
            std::vector<std::pair<std::string, int64_t> >* vec,
            Node* which=NULL, int num_retries=NUM_RETRIES);

    // Removes all elements in the sorted set stored at key with rank between start and stop.
    // Both start and stop are 0 -based indexes with 0 being the element with the lowest score.
    // These indexes can be negative numbers, where they indicate offsets starting at the element with the highest score.
    // For example:
    // -1 is the element with the highest score,
    // -2 the element with the second highest score and so forth.
    //
    // Time complexity:
    // O(log(N)+M) with N being the number of elements in the sorted set and M the number of elements removed by the operation.
    //
    // Return the number of elements removed.
    int zremrangebyrank(const std::string& key, int64_t start, int64_t end, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Determine the index of a member in a sorted set.
    // Time complexity: O(log(N))
    // Return -1 if field not exists
    int zrank(const std::string& key, const std::string& field, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Determine the index of a member in a sorted set, with scores ordered from high to low.
    int zrevrank(const std::string& key, const std::string& field, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Get the score associated with the given memer in a sorted set.
    // Time complexity: O(1)
    // Return -1 if field not exists
    int64_t zscore(const std::string& key, const std::string& field, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Incrementally iterate sorted sets elements and associated scores.
    //
    // Time complexity:
    // O(1) for every call. O(N) for a complete iteration,
    // including enough command calls for the cursor to return back to 0. N is the number of elements inside the collection..
    //
    // Returns an updated cursor that the user needs to use as the cursor argument in the next call.
    int64_t zscan(const std::string& key, int64_t cursor,
            std::vector<std::pair<std::string, int64_t> >* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    int64_t zscan(const std::string& key, int64_t cursor, int count,
            std::vector<std::pair<std::string, int64_t> >* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    int64_t zscan(const std::string& key, int64_t cursor, const std::string& pattern,
            std::vector<std::pair<std::string, int64_t> >* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);

    // If pattern is empty, then not using MATCH,
    // If count is 0, then not using COUNT
    int64_t zscan(const std::string& key, int64_t cursor, const std::string& pattern, int count,
            std::vector<std::pair<std::string, int64_t> >* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);

public: // STREAM (key like kafka's topic), available since 5.0.0.
    // Removes one or multiple messages from the pending entries list (PEL) of a stream consumer group.
    // The command returns the number of messages successfully acknowledged.
    int xack(const std::string& key, const std::string& groupname, const std::vector<std::string>& ids, Node* which=NULL, int num_retries=NUM_RETRIES);
    int xack(const std::string& key, const std::string& groupname, const std::string& id, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Returns the ID of the added entry. The ID is the one auto-
    // generated if * is passed as ID argument, otherwise the command just
    // returns the same ID specified by the user during insertion.
    //
    // c '~' or '='
    std::string xadd(const std::string& key, const std::string& id,
            const std::vector<FVPair>& values, int64_t maxlen, char c,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    std::string xadd(const std::string& key, const std::string& id,
            const std::vector<FVPair>& values,
            Node* which=NULL, int num_retries=NUM_RETRIES);

    // Create a new consumer group associated with a stream.
    // There are no hard limits to the number of consumer groups you can associate to a given stream.
    //
    // '$' the ID of the last item in the stream
    void xgroup_create(const std::string& key, const std::string& groupname,
            const std::string& id=std::string("$"), bool mkstream=false,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    void xgroup_destroy(const std::string& key, const std::string& groupname,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    void xgroup_setid(const std::string& key,
            const std::string& id=std::string("$"),
            Node* which=NULL, int num_retries=NUM_RETRIES);
    void xgroup_delconsumer(const std::string& key, const std::string& groupname, const std::string& consumername,
            Node* which=NULL, int num_retries=NUM_RETRIES);

    // Reads more than one keys
    // xread is a read command, can be called on slaves, xreadgroup is not
    void xread(const std::vector<std::string>& keys, const std::vector<std::string>& ids,
            int64_t count, int64_t block_milliseconds, std::vector<Stream>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    void xread(const std::vector<std::string>& keys, const std::vector<std::string>& ids,
            int64_t count, std::vector<Stream>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    void xread(const std::vector<std::string>& keys, const std::vector<std::string>& ids,
            std::vector<Stream>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);

    // Only read one key
    void xread(const std::string& key, const std::vector<std::string>& ids,
            int64_t count, int64_t block_milliseconds, std::vector<StreamEntry>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    // Use '>' as id
    void xread(const std::string& key,
            int64_t count, int64_t block_milliseconds, std::vector<StreamEntry>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);

    // Reads more than one keys
    // xreadgroup is not read command
    void xreadgroup(const std::string& groupname, const std::string& consumername,
            const std::vector<std::string>& keys, const std::vector<std::string>& ids,
            int64_t count, int64_t block_milliseconds, bool noack, std::vector<Stream>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    void xreadgroup(const std::string& groupname,
            const std::string& consumername, const std::vector<std::string>& keys,
            const std::vector<std::string>& ids, int64_t count, bool noack, std::vector<Stream>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    void xreadgroup(const std::string& groupname, const std::string& consumername,
            const std::vector<std::string>& keys, const std::vector<std::string>& ids,
            bool noack, std::vector<Stream>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);

    // Only read one key
    void xreadgroup(const std::string& groupname, const std::string& consumername,
            const std::string& key, const std::vector<std::string>& ids,
            int64_t count, int64_t block_milliseconds, bool noack, std::vector<StreamEntry>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    // Use '>' as id
    void xreadgroup(const std::string& groupname, const std::string& consumername,
            const std::string& key, int64_t count, int64_t block_milliseconds,
            bool noack, std::vector<StreamEntry>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);

    // Removes the specified entries from a stream, and returns the number of
    // entries deleted, that may be different from the number of IDs passed to the
    // command in case certain IDs do not exist.
    int xdel(const std::string& key, const std::vector<std::string>& ids, Node* which=NULL, int num_retries=NUM_RETRIES);
    int xdel(const std::string& key, const std::string& id, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Trims the stream to a given number of items, evicting older items (items with lower IDs) if needed
    // Returns the number of entries deleted from the stream
    int64_t xtrim(const std::string& key, int64_t maxlen, char c, Node* which=NULL, int num_retries=NUM_RETRIES);
    int64_t xtrim(const std::string& key, int64_t maxlen, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Returns the number of entries inside a stream. If the specified key does not
    // exist the command returns zero, as if the stream was empty.
    int64_t xlen(const std::string& key, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Returns the stream entries matching a given range of IDs
    // start The '-' special ID mean respectively the minimum ID possible inside a stream
    // end The '+' special ID mean respectively the maximum ID possible inside a stream
    void xrange(const std::string& key,
            const std::string& start, const std::string& end, int64_t count, std::vector<StreamEntry>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    void xrange(const std::string& key, const std::string& start, const std::string& end, std::vector<StreamEntry>* values, Node* which=NULL, int num_retries=NUM_RETRIES);
    void xrevrange(const std::string& key,
            const std::string& end, const std::string& start, int64_t count, std::vector<StreamEntry>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    void xrevrange(const std::string& key,
            const std::string& end, const std::string& start, std::vector<StreamEntry>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);

    // Fetching data from a stream via a consumer group,
    // and not acknowledging such data, has the effect of creating pending entries.
    //
    // Returns the number of pending messages
    int xpending(const std::string& key, const std::string& groupname,
            const std::string& start, const std::string& end, int count, const std::string& consumer,
            std::vector<struct DetailedPending>* pendings,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    int xpending(const std::string& key, const std::string& groupname,
            const std::string& start, const std::string& end, int count,
            std::vector<struct DetailedPending>* pendings,
            Node* which=NULL, int num_retries=NUM_RETRIES);

    // start the smallest ID among the pending messages
    // end the greatest ID among the pending messages
    // count the total number of pending messages for this consumer group
    // consumers the list of consumers in the consumer group with at least one pending message
    //
    // Returns the total number of pending messages for this consumer group
    int xpending(const std::string& key, const std::string& groupname, struct GroupPending* groups, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Gets ownership of one or multiple messages in the Pending Entries List
    // of a given stream consumer group.
    void xclaim(
            const std::string& key, const std::string& groupname, const std::string& consumer,
            int64_t minidle, const std::vector<std::string>& ids,
            int64_t idletime, int64_t unixtime, int64_t retrycount, bool force,
            std::vector<StreamEntry>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    void xclaim(
            const std::string& key, const std::string& groupname, const std::string& consumer,
            int64_t minidle, const std::vector<std::string>& ids,
            std::vector<StreamEntry>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    void xclaim(
            const std::string& key, const std::string& groupname, const std::string& consumer,
            int64_t minidle, const std::vector<std::string>& ids,
            int64_t idletime, int64_t unixtime, int64_t retrycount, bool force,
            std::vector<std::string>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);
    void xclaim(
            const std::string& key, const std::string& groupname, const std::string& consumer,
            int64_t minidle, const std::vector<std::string>& ids,
            std::vector<std::string>* values,
            Node* which=NULL, int num_retries=NUM_RETRIES);

    // Get the list of every consumer in a specific consumer group
    int xinfo_consumers(const std::string& key, const std::string& groupname, std::vector<struct ConsumerInfo>* infos, Node* which=NULL, int num_retries=NUM_RETRIES);
    // Get as output all the consumer groups associated with the stream
    int xinfo_groups(const std::string& key, std::vector<struct GroupInfo>* infos, Node* which=NULL, int num_retries=NUM_RETRIES);
    // Returns general information about the stream stored at the specified key
    void xinfo_stream(const std::string& key, struct StreamInfo* info, Node* which=NULL, int num_retries=NUM_RETRIES);

public: // BITMAP    
    // Sets or clears the bit on the specified offset for the string value stored by key.
    // Automatically generates a new string value when the key does not exist.
    //
    // Time complexity: O(1)
#if !defined(setbit)
    void setbit(const std::string& key, uint32_t offset, uint32_t value, Node* which=NULL, int num_retries=NUM_RETRIES);
#else
#undef setbit
    // <sys/param.h>
    // #define setbit(a,i) ((a)[(i)/NBBY] |= 1<<((i)%NBBY))
    void setbit(const std::string& key, uint32_t offset, uint32_t value, Node* which=NULL, int num_retries=NUM_RETRIES);
#endif // setbit

    // Gets the bit at the specified offset for the string value stored by key.
    // Return 0 if offset is larger than the length of the string value, or if key does not exist.
    //
    // Time complexity: O(1)
    int getbit(const std::string& key, uint32_t offset, Node* which=NULL, int num_retries=NUM_RETRIES);

    // Calculates the number of bits set to 1 in a given string. 
    // In general, the entire string given will be counted. 
    // By specifying an extra start or end parameter,the count can only be made on a specific bit.
    // In particular, the range indicated by the start and end parameters is a range of bytes, not a series of bit ranges.
    //
    // Time complexity: O(N)
    int bitcount(const std::string& key, int32_t start=0, int32_t end=-1,Node* which=NULL, int num_retries=NUM_RETRIES);

    // Returns the position of the first bit in the bitmap whose value is bit.
    // By default, the command will detect the entire bitmap, 
    // but the user can also specify the range to detect with the optional start and end parameters.
    // In particular, the range indicated by the start and end parameters is a range of bytes, not a series of bit ranges.
    //
    // Time complexity: O(N)
    int bitpos(const std::string& key, uint8_t bit, int32_t start=0, int32_t end=-1,Node* which=NULL, int num_retries=NUM_RETRIES);

public:
    // Standlone: key should be empty
    // Cluse mode: key used to locate node
    const RedisReplyHelper redis_command(
            bool readonly, int num_retries,
            const std::string& key, const CommandArgs& command_args,
            Node* which);

private:
    // 有些错误可安全无条件地重试，有些则需调用者决定是否重试，
    // 如果是网络连接断开错误，则还需要重建立连接
    //
    // 网络超时这样的错误：
    // incrby操作重试可能造成重复，
    // 但所有读操作重试都是安全的，类似于set这样覆盖写操作也是安全的，
    // 类似lpush这样的操作，也可能生成重试
    //
    // HR_SUCCESS 成功无需重试
    // HR_ERROR 明确错误无需重试
    // HR_RETRY_COND 有条件重试
    // HR_RETRY_UNCOND 需要无条件重试
    // HR_RECONN_COND 有条件重连接并重试
    // HR_RECONN_UNCOND 无条件重连接并重试
    // HR_REDIRECT 服务端返回ASK需要重定向
    enum HandleResult { HR_SUCCESS, HR_ERROR, HR_RETRY_COND, HR_RETRY_UNCOND, HR_RECONN_COND, HR_RECONN_UNCOND, HR_REDIRECT };

    // Handle the redis command error
    // Return -1 to break, return 1 to retry conditionally
    // 因为网络错误结果是未定义的，对于读操作一般可无条件的重试，对于写操作则需由调用者决定
    HandleResult handle_redis_command_error(CRedisNode* redis_node, const CommandArgs& command_args, struct ErrorInfo* errinfo);

    // Handle the redis reply
    // Success returns 0,
    // return -1 to break, return 2 to retry unconditionally
    // 对于明确的错误（如MOVED错误），可无条件地重试
    HandleResult handle_redis_reply(CRedisNode* redis_node, const CommandArgs& command_args, const redisReply* redis_reply, struct ErrorInfo* errinfo);
    HandleResult handle_redis_replay_error(CRedisNode* redis_node, const CommandArgs& command_args, const redisReply* redis_reply, struct ErrorInfo* errinfo);

private:
    void fini();
    void init();
    bool init_standlone(struct ErrorInfo* errinfo);
    bool init_cluster(struct ErrorInfo* errinfo);
    bool init_master_nodes(const std::vector<struct NodeInfo>& nodes_info, std::vector<struct NodeInfo>* replication_nodes_info, struct ErrorInfo* errinfo);
    void init_replica_nodes(const std::vector<struct NodeInfo>& replication_nodes_info);
    void update_slots(const struct NodeInfo& nodeinfo);
    void refresh_master_node_table(struct ErrorInfo* errinfo, const Node* error_node);
    void clear_and_update_master_nodes(const std::vector<struct NodeInfo>& nodes_info, std::vector<struct NodeInfo>* replication_nodes_info, struct ErrorInfo* errinfo);
    void clear_invalid_master_nodes(const NodeInfoTable& master_nodeinfo_table);
    bool add_master_node(const NodeInfo& nodeinfo, struct ErrorInfo* errinfo);
    void clear_all_master_nodes();
    void update_nodes_string(const NodeInfo& nodeinfo);
    redisContext* connect_redis_node(const Node& node, struct ErrorInfo* errinfo, bool readonly) const;
    CRedisNode* get_redis_node(int slot, bool readonly, const Node* ask_node, struct ErrorInfo* errinfo);
    CRedisMasterNode* get_redis_master_node(const NodeId& nodeid) const;
    CRedisMasterNode* random_redis_master_node() const;

private:
    // List the information of all cluster nodes
    bool list_cluster_nodes(std::vector<struct NodeInfo>* nodes_info, struct ErrorInfo* errinfo, redisContext* redis_context, const Node& node);

    // Called by: redis_command
    void extract_errtype(const redisReply* redis_reply, std::string* errtype) const;

public:
    // Called by: get,hget,key_type,lpop,rpop,srandmember
    bool get_value(const redisReply* redis_reply, std::string* value);

    // Called by: hkeys,hvals,lrange,mget,scan,smembers,spop,srandmember,sscan
    int get_values(const redisReply* redis_reply, std::vector<std::string>* values);

    // Called by: sscan
    int get_values(const redisReply* redis_reply, std::set<std::string>* values);

    // Called by: zrange & zrevrange & zrangebyscore & zrevrangebyscore & zscan
    int get_values(const redisReply* redis_reply, std::vector<std::pair<std::string, int64_t> >* vec, bool withscores);

    // Called by: hgetall & hscan
    int get_values(const redisReply* redis_reply, std::map<std::string, std::string>* map);

    // Called by: hmget
    int get_values(const redisReply* redis_reply, const std::vector<std::string>& fields, bool keep_null, std::map<std::string, std::string>* map);

    // Called by: hmincrby
    int get_values(const redisReply* redis_reply, std::vector<int64_t>* values);

public: // Stream
    // Called by: xreadgroup
    int get_values(const redisReply* redis_reply, std::vector<Stream>* values);

    // Called by xrange & xrevrange
    int get_values(const redisReply* redis_reply, std::vector<StreamEntry>* values);

    // Called by xpending
    int get_values(const redisReply* redis_reply, std::vector<struct DetailedPending>* pendings);

    // Called by xpending
    // Returns the total number of pending messages for this consumer group
    int get_values(const redisReply* redis_reply, struct GroupPending* groups);

    // Called by xinfo_consumers
    int get_values(const redisReply* redis_reply, std::vector<struct ConsumerInfo>* infos);

    // Called by xinfo_groups
    int get_values(const redisReply* redis_reply, std::vector<struct GroupInfo>* infos);

    // Called by xinfo_stream
    void get_value(const redisReply* redis_reply, struct StreamInfo* info);

    // Called by xinfo_stream
    void get_entry(const redisReply* entry_redis_reply, struct StreamEntry* entry);

public:
    void set_command_monitor(CommandMonitor* command_monitor) { _command_monitor = command_monitor; }
    CommandMonitor* get_command_monitor() const { return _command_monitor; }

private:
    bool _enable_debug_log; // Default: true
    bool _enable_info_log;  // Default: true
    bool _enable_error_log; // Default: true

private:
    CommandMonitor* _command_monitor;
    std::string _raw_nodes_string; // 最原始的
    std::string _nodes_string; // 长时间运行后，最原始的节点可能都不在了
    int _connect_timeout_milliseconds; // The connect timeout in milliseconds
    int _readwrite_timeout_milliseconds; // The receive and send timeout in milliseconds
    std::string _password;
    ReadPolicy _read_policy;

private:
    typedef std::unordered_map<Node, CRedisMasterNode*, NodeHasher> RedisMasterNodeTable;
    typedef std::unordered_map<NodeId, Node> RedisMasterNodeIdTable;

    RedisMasterNodeTable _redis_master_nodes; // Node -> CMasterNode
    RedisMasterNodeIdTable _redis_master_nodes_id; // NodeId -> Node

private:
    std::vector<Node> _nodes; // All nodes array
    std::vector<Node> _slot2node; // Slot -> Node

private:
    std::string _hincrby_shastr1;
    std::string _hmincrby_shastr1;
};

// Monitor the execution of the command by setting a CommandMonitor.
//
// Execution order:
// 1) before_command
// 2) command
// 3) after_command
class CommandMonitor
{
public:
    virtual ~CommandMonitor() {}

    // Called before each command is executed
    virtual void before_execute(const Node& node, const std::string& command, const CommandArgs& command_args, bool readonly) = 0;

    // Called after each command is executed
    // result The result of the execution of the command (0 success, 1 error, 2 timeout)
    virtual void after_execute(int result, const Node& node, const std::string& command, const redisReply* reply) = 0;
};

// Error code
enum
{
    ERROR_PARAMETER = -1,              // Parameter error
    ERROR_INIT_REDIS_CONN = -2,        // Initialize redis connection error
    ERROR_COMMAND = -3,                // Command error
    ERROR_CONNECT_REDIS = -4,          // Can not connect any cluster node
    ERROR_FORMAT = -5,                 // Format error
    ERROR_NOT_SUPPORT = -6,            // Not support
    ERROR_SLOT_NOT_EXIST = -7,         // Slot not exists
    ERROR_NOSCRIPT = -8,               // NOSCRIPT No matching script
    ERROR_UNKNOWN_REPLY_TYPE = -9,     // unknhown reply type
    ERROR_NIL = -10,                   // Redis return Nil
    ERROR_INVALID_COMMAND = -11,       // Invalid command
    ERROR_ZERO_KEY = -12,              // Key size is zero
    ERROR_REDIS_CONTEXT = -13,         // Can't allocate redis context
    ERROR_REDIS_AUTH = -14,             // Authorization failed
    ERROR_UNEXCEPTED_REPLY_TYPE = -15, // Unexcepted reply type
    ERROR_REPLY_FORMAT = -16,          // Reply format error
    ERROR_REDIS_READONLY = -17,
    ERROR_NO_ANY_NODE = -18
};

// Set NULL to discard log
typedef void (*LOG_WRITE)(const char* format, ...) __attribute__((format(printf, 1, 2)));
extern void set_error_log_write(LOG_WRITE error_log);
extern void set_info_log_write(LOG_WRITE info_log);
extern void set_debug_log_write(LOG_WRITE debug_log);

extern std::string strsha1(const std::string& str);
extern void debug_redis_reply(const char* command, const redisReply* redis_reply, int depth=0, int index=0);
extern uint16_t crc16(const char *buf, int len);
extern uint64_t crc64(uint64_t crc, const unsigned char *s, uint64_t l);

extern void millisleep(int milliseconds);
extern std::string get_formatted_current_datetime(bool with_milliseconds=false);
extern std::string format_string(const char* format, ...) __attribute__((format(printf, 1, 2)));
extern int split(std::vector<std::string>* tokens, const std::string& source, const std::string& sep, bool skip_sep=false);
extern int get_key_slot(const std::string* key);
extern bool keys_crossslots(const std::vector<std::string>& keys);
extern std::string int2string(int64_t n);
extern std::string int2string(int32_t n);
extern std::string int2string(int16_t n);
extern std::string int2string(uint64_t n);
extern std::string int2string(uint32_t n);
extern std::string int2string(uint16_t n);

// Convert a string into a int64_t. Returns true if the string could be parsed into a
// (non-overflowing) int64_t, false otherwise. The value will be set to the parsed value when appropriate.
extern bool string2int(const char* s, size_t len, int64_t* val, int64_t errval=-1);
extern bool string2int(const char* s, size_t len, int32_t* val, int32_t errval=-1);

} // namespace r3c {
#endif // REDIS_CLUSTER_CLIENT_H
