#pragma once

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


