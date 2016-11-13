//
//  main.cpp
//  Singaletim
//
//  Created by Atropos on 2016/11/11.
//  Copyright © 2016年 AnaBit. All rights reserved.
//

#include <map>
#include <mutex>
#include <cassert>
#include <memory>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <functional>

class Signaletim {
public:
    using Signal = const void *;
    
public:
    Signaletim();
    ~Signaletim();
    
    template <typename T>
    static const T & conv(const void * ptr) {
        return *((T *) ptr);
    }
    
    bool regist(const std::string & key, std::function<void(Signal)> && func);
    
    template <typename T>
    bool post(const std::string & key, T && signal);
    
    bool remove(const std::string & key);
    
private:
    static std::map<std::string, std::vector<Signaletim *>> _centerRegist;
    static std::mutex _lock;
    
private:
    std::map<std::string, std::function<void(Signal)>> _localRegist;
};

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

bool Signaletim::regist(const std::string &key, std::function<void (Signaletim::Signal)> && func)
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

template <typename T>
bool Signaletim::post(const std::string & key, T && signal)
{
    std::shared_ptr<T> share = std::make_shared<T>(std::move(signal));
    {
        std::unique_lock<std::mutex> lock(_lock);
        auto find = _centerRegist.find(key);
        if (find == _centerRegist.end()) {
            return false;
        }
        
        auto Signals = find->second;
        for (auto & signal : Signals) {
            auto pairFunc = signal->_localRegist.find(key);
            assert(pairFunc != signal->_localRegist.end());
            (pairFunc->second)(share.get());
        }
    }
    
    return true;
}

bool Signaletim::remove(const std::string &key)
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


struct Device {
    std::string name;
    std::string ip;
    std::string version;
};

int main(int argc, const char * argv[]) {
    // insert code here...
    
    Signaletim signal;
    signal.regist("hello", [] (Signaletim::Signal signal) {
        std::cout << "dev name = " << Signaletim::conv<Device>(signal).name << std::endl
        << "      ip = " << Signaletim::conv<Device>(signal).ip << std::endl
        << " version = " << Signaletim::conv<Device>(signal).version << std::endl;
    } );
    
    Device dev {"phone", "192.168.51.41", "verison"};
    std::cout << dev.name << std::endl;
    signal.post("hello", std::move(dev));
    
    return 0;
}
