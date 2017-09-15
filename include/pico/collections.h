#ifndef PICOLIB_COLLECTIONS_H_
#define PICOLIB_COLLECTIONS_H_

#include <initializer_list>
#include <utility>
#include <type_traits>

namespace Pico {

    template <typename T>
    class Collection
    {
        public:
            METHOD size_t   length() const { return nr_elements; }
            METHOD T&       empty() const { return nr_elements == 0; }
            METHOD T&       first() const { return &storage[0]; }
            METHOD T&       last() const { return &storage[nr_elements - 1]; }
            METHOD T&       operator [](unsigned index) {
                return const_cast<T&>(static_cast<const Collection*>(this)->operator[](index));
            }

            METHOD const T& operator [](unsigned index) const {
                assert(index < nr_elements);
                return storage[index];
            }

            METHOD int      index(T const& val) const {
                for ( size_t i = 0; i < nr_elements; i++ )
                    if ( storage[i] == val )
                        return i;

                return -1;
            }

            template <typename Func>
            METHOD bool     any(Func proc) const {
                for ( T const& e : *this )
                    if ( proc(e) == true )
                        return true;

                return false;
            }

            template <typename Func>
            METHOD bool     all(Func proc) const {
                for ( T const& e : *this )
                    if ( proc(e) == false )
                        return false;

                return true;
            }

            METHOD bool     operator ==(Collection const& o) const {
                if ( this->length() != o.length() )
                    return false;

                for ( size_t i = 0; i < this->length(); i++ )
                    if ( (*this)[i] != o[i] )
                        return false;

                return true;
            }

            METHOD bool     operator !=(Collection const& o) const {
                return !(*this == o);
            }

            METHOD T*       begin() { return &storage[0]; }
            METHOD T*       end() { return &storage[nr_elements]; }

        protected:
            CONSTRUCTOR Collection() = default;
            CONSTRUCTOR Collection(T *elements, size_t nr_elements) : storage(elements), nr_elements(nr_elements) {}
            CONSTRUCTOR Collection(Collection&& o) : storage(o.storage), nr_elements(o.nr_elements) {
                o.storage = nullptr;
                o.nr_elements = 0;
            }

            T *storage = nullptr;
            size_t nr_elements = 0;
    };

    template <typename T, unsigned Size>
    class Array : public Collection<T>
    {
        static_assert(Size > 0, "Pico::Array must have at least one element.");

        public:
            template <typename... V>
            CONSTRUCTOR Array(V&&... values) : Collection<T>(elements, Size), elements{ std::forward<V>(values)... } {}

            METHOD T* pointer() const { return elements; }

        private:
            T elements[Size];
    };

    template <typename T>
    class List : public Collection<T>
    {
        static constexpr size_t default_capacity = 10;

        public:
            CONSTRUCTOR List(Pico::Heap& heap = Pico::Heap::global(), size_t capacity = default_capacity) : heap(heap) {
                resize(capacity);
            }

            CONSTRUCTOR List(List&& o) : Collection<T>(std::move(o)), heap(o.heap), current_capacity(o.current_capacity)
            {
                o.current_capacity = 0;
            }

            DESTRUCTOR ~List() {
                clear();
                resize(0);
            }

            METHOD List& clear();
            METHOD size_t capacity() const { return current_capacity; }

            template <typename... V>
            METHOD List& insert(int pos, V&&... values);

            METHOD List& remove(int pos, size_t length);
            METHOD List slice(int pos, size_t length);

            template <typename... V>
            METHOD List& push(V&&... values);

            METHOD T pop();
            METHOD List<T> pop(size_t n);

        private:
            Pico::Heap& heap;
            size_t current_capacity = 0;

            template <typename... V>
            METHOD void create_elements(unsigned offset, V&&... values);
            METHOD void move_elements(unsigned from, unsigned to);
            METHOD void resize(size_t new_capacity);
            METHOD unsigned pos_to_offset(int pos) {
                if ( pos < 0 )
                    pos = this->nr_elements + pos;

                assert(pos >= 0 && static_cast<size_t>(pos) <= this->nr_elements);
                return pos;
            }
    };

    template <typename T>
    METHOD
    List<T>& List<T>::clear()
    {
        return remove(0, this->nr_elements);
    }

    template <typename T>
    template <typename... V>
    METHOD
    List<T>& List<T>::insert(int pos, V&&... values)
    {
        unsigned offset = pos_to_offset(pos);
        constexpr size_t nr_vals = sizeof...(values);
        const size_t required_capacity = nr_vals + this->nr_elements;

        // Ensure with a have a minimum capacity to store all the values.
        if ( required_capacity > current_capacity )
            resize(required_capacity + default_capacity);

        // Displace elements to make some room for the new values.
        if ( offset < this->nr_elements )
            move_elements(offset, offset + nr_vals);

        create_elements(offset, std::forward<T>(values)...);

        return *this;
    }

    template <typename T>
    METHOD
    List<T> List<T>::slice(int pos, size_t length)
    {
        unsigned offset = pos_to_offset(pos);

        if ( length > this->nr_elements - offset )
            length = this->nr_elements - offset;

        List<T> sliced(heap, length);

        for ( size_t i = 0; i < length; i++ )
            new (&sliced.storage[i]) T( std::move(this->storage[offset + i]) );

        sliced.nr_elements = length;
        remove(offset, length);

        return sliced;
    }

    template <typename T>
    METHOD
    List<T>& List<T>::remove(int pos, size_t length)
    {
        unsigned offset = pos_to_offset(pos);

        if ( length > this->nr_elements - offset )
            length = this->nr_elements - offset;

        for ( size_t i = 0; i < length; i++ )
            this->storage[offset + i].~T();

        if ( offset + length != this->nr_elements )
            move_elements(offset + length, offset);

        this->nr_elements -= length;

        return *this;
    }

    template <typename T>
    template <typename... V>
    METHOD
    List<T>& List<T>::push(V&&... values)
    {
        return insert(this->nr_elements, std::forward<V>(values)...);
    }

    template <typename T>
    METHOD
    T List<T>::pop()
    {
        T value = std::move(this->last());

        remove(this->nr_elements - 1, 1);
        return value;
    }

    template <typename T>
    METHOD
    List<T> List<T>::pop(size_t n)
    {
        if ( n > this->nr_elements )
            n = this->nr_elements;

        return slice(this->nr_elements - n, n);
    }

    template <typename T>
    template <typename... V>
    METHOD
    void List<T>::create_elements(unsigned offset, V&&... values)
    {
        constexpr size_t nr_vals = sizeof...(values);

        new (&this->storage[offset]) T[nr_vals] { std::forward<V>(values)... };
        this->nr_elements += nr_vals;
    }

    template <typename T>
    METHOD
    void List<T>::move_elements(unsigned from, unsigned to)
    {
        const int nr_moved = this->nr_elements - from;

        assert(nr_moved >= 0 && to + nr_moved <= current_capacity);

        if ( from == to )
            return;

        if ( from > to )
        {
            for ( int i = 0; i < nr_moved; i++ ) {
                new (&this->storage[to + i]) T( std::move(this->storage[from + i]) );
                this->storage[from + i].~T();
            }
        }
        else
        {
            for ( int i = 0; i < nr_moved; i++ ) {
                new (&this->storage[to + nr_moved - i - 1]) T( std::move(this->storage[from + nr_moved - i - 1]) );
                this->storage[from + nr_moved - i - 1].~T();
            }
        }
    }

    template <typename T>
    METHOD
    void List<T>::resize(size_t new_capacity)
    {
        if ( new_capacity == current_capacity )
            return;

        if ( this->storage == nullptr ) {
            this->storage = static_cast<T *>(heap.allocate(new_capacity * sizeof(T)));
            current_capacity = new_capacity;
            return;
        }

        assert(this->nr_elements <= new_capacity);

        if ( new_capacity == 0 ) {
            heap.free(this->storage, current_capacity * sizeof(T));
            this->storage = nullptr;
        }
        else {
            T *new_storage = static_cast<T *>(heap.allocate(new_capacity * sizeof(T)));

            for ( size_t i = 0; i < this->nr_elements; i++ ) {
                new (&new_storage[i]) T( std::move(this->storage[i]) );

                this->storage[i].~T();
            }

            heap.free(this->storage);
            this->storage = new_storage;
        }

        current_capacity = new_capacity;
    }

    //
    // Tiny tuple implementation.
    //
    template <typename... T>
    struct PACKED Tuple;

    template <>
    struct PACKED Tuple<> {};

    template <typename Head, typename... Rest>
    struct PACKED Tuple<Head, Rest...>
    {
        public:
            CONSTRUCTOR Tuple() {}
            CONSTRUCTOR Tuple<Head, Rest...>(Head v, Rest... r) : element(v), rest(r...) {}
            CONSTRUCTOR Tuple<Head, Rest...>(Tuple<Head, Rest...> const& o) : element(o.get<0>()), rest(o.tail()) {}

            template <unsigned I, typename T = typename std::enable_if<I == 0>::type>
            METHOD Head get() const { return element; }

            template <unsigned I, typename T = typename std::enable_if<I != 0>::type>
            METHOD auto get() const { return rest.template get<I-1>(); }

        private:
            Head element;
            Tuple<Rest...> rest;

            METHOD Tuple<Rest...> tail() const { return rest; }
    };
}

#endif
