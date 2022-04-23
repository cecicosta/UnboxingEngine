#pragma once

#include <iterator>
#include <type_traits>
#include <typeinfo>
#include <unordered_map>
#include <vector>
#include <functional>

#include <iostream>

class CEventDispatcher;

/// Base class for UListener variadic template implementation. Holds the hash_handles member std::vector.
/// \tparam Types Packed variadic types.
template<typename... Types>
class UListener {
public:
    virtual ~UListener() = default;
    friend CEventDispatcher;
protected:
    std::vector<size_t> hash_handles;
};

/// Listener template classes. Derive from each class given as template parameter,
/// allowing dynamic conversion to each specific class type.
/// Also keep the hash information of each class it derives from in its base class
/// member UListener::hash_handles.
/// \tparam T Variadic type unpacked template parameter. Represents the left-to-right
/// unpacked type given to UListener template.
/// \tparam Types Packed variadic types. Represents the types still to be resolved by
/// UListener derivation.
template<typename T, typename... Types>
class UListener<T, Types...> : public T, public UListener<Types...> {
public:
    /// Copy constructor. Can convert a given base class Listener type, to type T, if T is a derived UListener type.
    /// \param base
    explicit UListener(UListener<> *base) : UListener(dynamic_cast<T *>(base)) {}

    /// Default constructor. Fills in the base member UListener::hash_handles with the calculated type T hash.
    UListener() {
        UListener::hash_handles.emplace_back(typeid(T).hash_code());
    }
    ~UListener() override = default;
};

/// Template container derived from std::vector. Allows to access its In type elements as Out type elements if Out is dynamically convertible to In.
/// \tparam In Stored type elements.
/// \tparam Out Output type elements
template<typename In, typename Out = In,
         std::enable_if_t<std::is_pointer<In>::value && std::is_pointer<Out>::value, bool> = true>
class u_convertible_vector : public std::vector<In>{
    /// Derived iterator from vector<In>::iterator. Overloads the deference operators to convert dynamically the object type to T.
    template <typename T>
    struct u_convertible_iterator : std::vector<In>::iterator {
    public:
        T operator*() {return dynamic_cast<T>(std::vector<In>::iterator::operator*());}
        T operator->() {return dynamic_cast<T>(std::vector<In>::iterator::operator->());}
    };

public:

    u_convertible_vector() : std::vector<In>() {
        this->begin = [this](){
            return u_convertible_iterator<Out>{std::vector<In>::begin()};
        };
        this->end = [this](){
            return u_convertible_iterator<Out>{std::vector<In>::end()};
        };
    };

    /// Defines a range constructor.
    /// \param begin Iterator pointing to the first element of type In in the container
    /// \param end Iterator pointing to the last+1 element of type In in the container
    u_convertible_vector(typename std::vector<In>::iterator begin,
                          typename std::vector<In>::iterator end) : std::vector<In>(begin, end) {
        this->begin = [this](){
            return u_convertible_iterator<Out>{std::vector<In>::begin()};
        };
        this->end = [this](){
            return u_convertible_iterator<Out>{std::vector<In>::end()};
        };
    }

    /// Copy constructor
    /// \param original original u_convertible_vector which the new convertible vector will be build from
    explicit u_convertible_vector(const u_convertible_vector<In>& original) : std::vector<In>(original) {
        this->begin = [this](){
            return u_convertible_iterator<Out>{std::vector<In>::begin()};
        };
        this->end = [this](){
            return u_convertible_iterator<Out>{std::vector<In>::end()};
        };
    }

    ~u_convertible_vector() = default;

    ///Access to begin iterator of the this container dynamically converted to Out type
    std::function<u_convertible_iterator<Out>()> begin;
    ///Access to end iterator of the this container dynamically converted to Out type
    std::function<u_convertible_iterator<Out>()> end;
};

class CEventDispatcher {
    typedef UListener<> CListener;

public:
    CEventDispatcher() = default;
    virtual ~CEventDispatcher() = default;

protected:
    /// Register @listener to multiple events based on the types inherited by UListener<T, Types...>
    /// @param listener
    template<class T,
             std::enable_if_t<std::is_constructible<T, UListener<>>::value, bool> = true>
    void RegisterListener(T &listener) {
        for (auto &&hash: listener.hash_handles) {
            if (m_Listeners.find(hash) == m_Listeners.end()) {
                u_convertible_vector<CListener*, T*> first;
                first.push_back(&listener);
                m_Listeners.insert_or_assign(hash, first);
            } else {
                m_Listeners[hash].push_back(&listener);
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
    u_convertible_vector<CListener*, T*> GetListeners() {
        auto hash = typeid(T).hash_code();
        if (auto &&it = m_Listeners.find(hash); it != m_Listeners.end()) {
            return u_convertible_vector<CListener*, T*>(it->second);
        }
        return {};
    }

private:
    std::unordered_map<std::size_t, u_convertible_vector<CListener*>> m_Listeners;
};
