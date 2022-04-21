#pragma once

#include <iterator>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <vector>

#include <iostream>

class IStartListener;

template<typename... Types>
class IListener {
public:
    virtual ~IListener() = default;
    std::vector<size_t> hash_handles;
};

template<typename T, typename... Types>
class IListener<T, Types...> : public T, public IListener<Types...> {
public:
    template<typename... Args>
    friend class IListener;

    explicit IListener(IListener<> *base) : IListener(dynamic_cast<T *>(base)) {}

    IListener() {
        IListener::hash_handles.emplace_back(typeid(T).hash_code());
        std::cout << "Instantiation of: " << typeid(T).name() << std::endl;
    }
    ~IListener() override = default;
};


/// class template which creates a new vector of specialized-type elements from a container of a base-type.
/// \tparam From source container elements type
/// \tparam To type of the new container elements
template<typename From, typename To=From>
class to_specialized_vector {
    /// specialized-type iterator inheriting from vector<To>::iterator, extends the constructor options to
    /// allow building a new iterator from the existing base-type iterator
    template <typename T>
    struct to_specialized_iterator : std::vector<From>::iterator {
    public:
        T operator*() {return dynamic_cast<T>(std::vector<From>::iterator::operator*());}
        T operator->() {return dynamic_cast<T>(std::vector<From>::iterator::operator->());}
    };
public:
    std::vector<From> content;
    /// Defines a range constructor allowing for fast conversion from container base-type to the new container specialized-type
    /// \param begin Iterator pointing to the first elemexnt of the base-type container
    /// \param end Iterator pointing to the last+1 element of the base-type container
    to_specialized_vector(typename std::vector<From>::iterator begin,
                          typename std::vector<From>::iterator end)
        : content(std::vector<From>(begin, end)) {
    }

    explicit to_specialized_vector(const to_specialized_vector<From>& cpy)
        : content(cpy.content) {
    }

    to_specialized_vector() = default;
    ~to_specialized_vector() = default;
    /// Implements the standard library method for accessing the container iterator for the first element.
    /// \return iterator pointing to the first element of the vector.
    template <typename T>
    to_specialized_iterator<T> begin() { return to_specialized_iterator<T>{content.begin()}; }
    to_specialized_iterator<To> begin() { return begin<To>(); }
    /// Implements the standard library method for accessing the container iterator for the last+1 element.
    /// \return iterator pointing to the last+1 element of the vector.
    template <typename T>
    to_specialized_iterator<T> end() { return to_specialized_iterator<T>{content.end()}; }
    to_specialized_iterator<To> end() { return end<To>(); }
};

class CEventDispatcher {
    typedef IListener<> CListener;

public:
    CEventDispatcher() = default;
    virtual ~CEventDispatcher() = default;

protected:
    /// Register @listener to multiple events based on the types inherited by IListener<T, Types...>
    /// @param listener
    void RegisterListener(CListener &listener) {
        for (auto &&hash: listener.hash_handles) {
            if (m_Listeners.find(hash) == m_Listeners.end()) {
                to_specialized_vector<CListener*> first;
                first.content.push_back(&listener);
                m_Listeners[hash] = first;
            } else {
                m_Listeners[hash].content.push_back(&listener);
            }
        }
    }

    void UnregisterListener(CListener &listener) {
        for (auto &&hash: listener.hash_handles) {
            if (auto it = m_Listeners.find(hash); it != m_Listeners.end()) {
                m_Listeners.erase(it);
            }
        }
    }

    template<typename T>
    to_specialized_vector<CListener*, T*> GetListeners() {
        auto hash = typeid(T).hash_code();
        if (auto &&it = m_Listeners.find(hash); it != m_Listeners.end()) {
            return to_specialized_vector<CListener*, T*>(it->second);
        }
        return {};
    }

private:
    std::unordered_map<std::size_t, to_specialized_vector<CListener*>> m_Listeners;
};
