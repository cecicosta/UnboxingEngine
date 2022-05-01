#pragma once

#include "CustomTypes.h"

#include <iostream>
#include <typeinfo>
#include <unordered_map>

namespace unboxing_engine {

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

    class CEventDispatcher {
        typedef UListener<> CListener;

    public:
        CEventDispatcher() = default;
        virtual ~CEventDispatcher() = default;

        /// Register @listener to multiple events based on the types inherited by UListener<T, Types...>
        /// @param listener
        template<class T,
                 std::enable_if_t<std::is_convertible<T, CListener>::value, bool> = true>
        void RegisterListener(T &listener) {
            for (auto &&hash: listener.hash_handles) {
                if (m_Listeners.find(hash) == m_Listeners.end()) {
                    custom_types::u_convertible_vector<CListener*> first;
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
        custom_types::u_convertible_vector<CListener *, T *> GetListeners() {
            auto hash = typeid(T).hash_code();
            if (auto &&it = m_Listeners.find(hash); it != m_Listeners.end()) {
                return custom_types::u_convertible_vector<CListener *, T *>(it->second);
            }
            return {};
        }

    private:
        std::unordered_map<std::size_t, custom_types::u_convertible_vector<CListener *>> m_Listeners;
    };
}// namespace unboxing_engine