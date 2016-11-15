#include "signalemit.h"

std::map<std::string, std::vector<Signaletim *>> Signaletim::_centerRegist;
std::mutex Signaletim::_lock;

Signaletim::Signaletim()
{

}

Signaletim::~Signaletim()
{
    {
        std::unique_lock<std::mutex> lock(_lock);
        for (auto & each : _localRegist) {
            auto centerFind = _centerRegist.find(each.first);
            assert(centerFind != _centerRegist.end());
            centerFind->second.erase(std::find(centerFind->second.begin(), centerFind->second.end(), this));
        }
    }
}

bool Signaletim::regist(const std::__cxx11::string & key, std::function<void (Signaletim::Signal)> && func)
{
    {
        std::unique_lock<std::mutex> lock(_lock);

        auto find = _localRegist.find(key);
        if(find != _localRegist.end()) {
            return false;
        }

        _localRegist.emplace(key, std::move(func));

        auto reg = _centerRegist.find(key);
        if (reg == _centerRegist.end()) {
            _centerRegist.emplace(key, std::vector<Signaletim *>{ this });
            _localRegist.emplace(key, std::move(func));
        } else {
            reg->second.push_back(this);
        }
    }
    return true;
}

bool Signaletim::remove(const std::__cxx11::string & key)
{
    {
        std::unique_lock<std::mutex> lock(_lock);
        auto localFind = _localRegist.find(key);
        if (localFind == _localRegist.end()){
            return false;
        }

        _localRegist.erase(localFind);

        auto centerFind = _centerRegist.find(key);
        assert(centerFind != _centerRegist.end());
        centerFind->second.erase(std::find(centerFind->second.begin(), centerFind->second.end(), this));
    }
    return true;
}
