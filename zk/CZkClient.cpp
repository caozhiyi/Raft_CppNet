#include "CZkClient.h"
#include "Log.h"

#define BUF_SIZE 1024*5   //5kb

CZkClient::CZkClient():
    _zk_handle(nullptr),
    _is_connected(false) {

}

CZkClient::CZkClient(std::string host_name, long long time_out):
_host_name(host_name), 
_time_out(time_out),
_zk_handle(nullptr),
_is_connected(false) {

}

CZkClient::~CZkClient() {
    if (_zk_handle) {
        int ret = zookeeper_close(_zk_handle);
        if (ret != ZOK) {
            LOG_ERROR("Close zk handle field, error code: %d", ret);
        }
    }
}

bool CZkClient::ConnectZK() {
    _zk_handle = zookeeper_init(_host_name.c_str(), CZkClient::Watcher, _time_out, nullptr, this, 0);
    if (_zk_handle == nullptr) {
        LOG_ERROR("Connect zookeeper field");
        _is_connected = false;
    }
    _is_connected = true;
    return _is_connected;
}

bool CZkClient::ConnectZK(std::string host_name, long long time_out) {
    _host_name = host_name;
    _time_out = time_out;
    return ConnectZK();
}

bool CZkClient::CreateNode(const std::string& path, const std::string& value, bool temp) {
    if (!_is_connected) {
        bool ret = ConnectZK();
        if (ret == false) {
            return false;
        }
    }
    
    int ret = zoo_create(_zk_handle, path.c_str(), value.c_str(), value.length(), 
        &ZOO_OPEN_ACL_UNSAFE, temp ? ZOO_EPHEMERAL : 0, nullptr, 0);
    if (ret != ZOK) {
        LOG_ERROR("Create zookeeper node field, ret : %d, path : %s, value : %s", ret, path.c_str(), value.c_str());
        return false;
    }
    return true;
}

bool CZkClient::DeleteNode(const std::string& path, int version) {
    if (!_is_connected) {
        bool ret = ConnectZK();
        if (ret == false) {
            return false;
        }
    }

    int ret = zoo_delete(_zk_handle, path.c_str(), version);
    if (ret != ZOK) {
        LOG_ERROR("Delete zookeeper node field, ret : %d, path : %s", ret, path.c_str());
        return false;
    }
    return true;
}

int CZkClient::NodeExista(const std::string& path, bool watch) {
    if (!_is_connected) {
        bool ret = ConnectZK();
        if (ret == false) {
            return false;
        }
    }

    int ret = zoo_exists(_zk_handle, path.c_str(), watch?1:0, nullptr);
    return ret;
}

bool CZkClient::GetNodeValue(const std::string& path, std::string& value, bool watch) {
    if (!_is_connected) {
        bool ret = ConnectZK();
        if (ret == false) {
            return false;
        }
    }

    char buf[BUF_SIZE] = { 0 };
    int len = BUF_SIZE;
    int ret = zoo_get(_zk_handle, path.c_str(), watch?1:0, buf, &len, nullptr);
    if (ret != ZOK) {
        LOG_ERROR("Get zookeeper node value field, ret : %d, path : %s", ret, path.c_str());
        return false;
    }
    value = buf;
    return true;
}

bool CZkClient::SetNodeValue(const std::string& path, const std::string& value, int version) {
    if (!_is_connected) {
        bool ret = ConnectZK();
        if (ret == false) {
            return false;
        }
    }

    int ret = zoo_set(_zk_handle, path.c_str(), value.c_str(), value.length(), version);
    if (ret != ZOK) {
        LOG_ERROR("Set zookeeper node value field, ret : %d, path : %s, value : %s", ret, path.c_str(), value.c_str());
        return false;
    }
    return true;
}

bool CZkClient::GetAllChildren(const std::string& path, std::map<std::string, std::string>& ret_map, bool watch) {
    if (!_is_connected) {
        bool ret = ConnectZK();
        if (ret == false) {
            return false;
        }
    }

    struct String_vector str_vec;
    int ret = zoo_get_children(_zk_handle, path.c_str(), watch?1:0, &str_vec);
    if (ret != ZOK) {
        LOG_ERROR("Get zookeeper children field, ret : %d, path : %s", ret, path.c_str());
        return false;
    }

    for (int i = 0; i < str_vec.count; i++) {
        std::string node = str_vec.data[i];
        std::string path = path + "/" + node;
        std::string value = "";
        if (GetNodeValue(path, value)) {
            ret_map[node] = value;

        } else {
            LOG_ERROR("Get zookeeper children field, ret : %d, path : %s", ret, path.c_str());
        }
    }
    return true;
}

CallBackFunc CZkClient::SetCallBack(const CallBackFunc& func) {
    CallBackFunc ret_func = _call_back;
    _call_back = func;
    return ret_func;
}

void CZkClient::Watcher(zhandle_t *zh, int type, int state, const char *path, void *watcherCtx) {
    CZkClient* client = static_cast<CZkClient*>(watcherCtx);

    if (type == ZOO_SESSION_EVENT) {
        if (state == ZOO_CONNECTED_STATE) {
            client->_is_connected = true;
            LOG_ERROR("Connected to zookeeper");
        } else {
            client->_is_connected = false;
            LOG_ERROR("Disconnected to zookeeper, state: %d", state);
        }
       
    } else {
        if (client->_call_back){
            client->_call_back(type, path);
        }
    }
}
