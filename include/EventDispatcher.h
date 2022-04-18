#pragma once

#include <typeinfo>
#include <unordered_map>
#include <vector>

#include <iostream>

template<typename... Types>
class IListener {
public:
    virtual ~IListener() = default;
    std::vector<size_t> hash_handles;
};

template<typename T, typename... args>
class IListener<T, args...> : public T, public IListener<args...> {
public:
    IListener() {
        IListener::hash_handles.emplace_back(typeid(T).hash_code());
        std::cout << "Instantiation of: " << typeid(T).name() << std::endl;
    }
    ~IListener() override = default;
};

class CEventDispatcher {
public:
    CEventDispatcher() = default;
    virtual ~CEventDispatcher() = default;

protected:
    void RegisterListener(IListener<> &listener) {
        for (auto &&hash: listener.hash_handles) {
            if (m_Listeners.find(hash) == m_Listeners.end()) {
                std::vector<IListener<> *> first;
                first.push_back(&listener);
                m_Listeners[hash] = first;
            } else {
                m_Listeners[hash].push_back(&listener);
            }
        }
    }

    void UnregisterListener(IListener<> &listener) {
        for (auto &&hash: listener.hash_handles) {
            if (auto it = m_Listeners.find(hash); it != m_Listeners.end()) {
                m_Listeners.erase(it);
            }
        }
    }

    template<typename T>
    std::vector<T*> GetListeners() {
        auto hash = typeid(T).hash_code();
        if (auto it = m_Listeners.find(hash); it != m_Listeners.end()) {
            std::vector<T*> converted;
            for (auto &&listener: it->second) {
                converted.push_back(dynamic_cast<T*>(listener));
            }
            return converted;
        }
        return std::vector<T*>();
    }

private:
    std::unordered_map<std::size_t, std::vector<IListener<> *>> m_Listeners;
};
