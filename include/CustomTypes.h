#pragma once

#include <iterator>
#include <type_traits>
#include <vector>
#include <functional>

namespace unboxing_engine::custom_types {

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
}