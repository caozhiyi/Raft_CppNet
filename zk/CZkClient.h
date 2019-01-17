#ifndef HEADER_ZK_CLIENT
#define HEADER_ZK_CLIENT

#include <functional>
#include <map>
#include "Single.h"
#include "zookeeper.h"
enum EVENT_TYPE {
    CREATED_EVENT_DEF = 1,
    DELETED_EVENT_DEF = 2,
    CHANGED_EVENT_DEF = 3,
    CHILD_EVENT_DEF = 4,
    NOTWATCHING_EVENT_DEF = -2
};
typedef std::function<void(int, std::string)> CallBackFunc;

class CZkClient: public CSingle<CZkClient> {
public:
    CZkClient();
    CZkClient(std::string host_name, long long time_out);
    ~CZkClient();
    
    bool ConnectZK();
    bool ConnectZK(std::string host_name, long long time_out);
    bool CreateNode(const std::string& path, const std::string& value, bool temp = false);
    bool DeleteNode(const std::string& path, int version = -1);

    /*
    * return value means:
    * ZOK operation completed successfully
    * ZNONODE the node does not exist.
    * ZNOAUTH the client does not have permission.
    * ZBADARGUMENTS - invalid input parameters
    * ZINVALIDSTATE - zhandle state is either ZOO_SESSION_EXPIRED_STATE or ZOO_AUTH_FAILED_STATE
    * ZMARSHALLINGERROR - failed to marshall a request; possibly, out of memory
    */
    int NodeExista(const std::string& path, bool watch = false);
    bool GetNodeValue(const std::string& path, std::string& value, bool watch = false);
    bool SetNodeValue(const std::string& path, const std::string& value, int version=-1);
    bool GetAllChildren(const std::string& path, std::map<std::string, std::string>& ret_map, bool watch = false);
    CallBackFunc SetCallBack(const CallBackFunc& func);

private:
    static void Watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx);

private:
    std::string        _host_name;
    zhandle_t*         _zk_handle;
    long long          _time_out;
    bool               _is_connected;
    CallBackFunc       _call_back;
};

#endif