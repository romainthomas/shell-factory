#ifndef PICOLIB_STRING_H_
#define PICOLIB_STRING_H_

#include <cstdarg>

namespace Pico {

    template <typename T>
    class BasicString
    {
        public:
            static constexpr size_t npos = -1;

            template <unsigned N>
            CONSTRUCTOR BasicString(const T (&src)[N])
            {
                chars = const_cast<T *>(src);
                max_size = size = N;
            }

            CONSTRUCTOR BasicString();
            CONSTRUCTOR BasicString(size_t buffer_size)
            {
                chars = new T[buffer_size];
                chars[0] = 0;
                needs_dealloc = true;

                size = 1;
                max_size = buffer_size;
            }

            CONSTRUCTOR BasicString(T* src)
            {
                chars = src;
                max_size = size = length(chars) + 1;
            }


            CONSTRUCTOR BasicString(const T* src) :
              BasicString{src, length(src) + 1}
            {
            }


            CONSTRUCTOR BasicString(const T* src, size_t buffer_size) :
              BasicString{buffer_size}
            {

                BasicString<T>::copy(chars, src);
                chars[buffer_size - 1] = 0;

                max_size = buffer_size;
                size = length(chars) + 1;
                needs_dealloc = true;
            }

            CONSTRUCTOR BasicString(T* src, size_t buffer_size, bool auto_free = false)
            {
                chars = src;
                chars[buffer_size - 1] = 0;

                max_size = buffer_size;
                size = length(chars) + 1;
                needs_dealloc = auto_free;
            }

            //
            // Copy constructor.
            //
            CONSTRUCTOR BasicString(BasicString<T>& o)
            {
                chars = new T[o.max_size];
                needs_dealloc = true;

                BasicString<T>::copy(chars, o.chars);
                size = o.size;
                max_size = o.max_size;
            }

            //
            // Move constructor.
            //
            CONSTRUCTOR BasicString(BasicString<T>&& o) : chars(o.chars), size(o.size), max_size(o.max_size), needs_dealloc(o.needs_dealloc)
            {
                o.chars = nullptr;
                o.size = o.max_size = 0;
            }

            DESTRUCTOR ~BasicString()
            {
                if ( needs_dealloc )
                    delete[] chars;
            }

            METHOD T *pointer() const { return chars; }
            METHOD size_t length() const { return size - 1; }
            METHOD bool empty() const { return length() == 0; }

            METHOD T& operator[](int index) const {
                if ( index < 0 )
                    index = length() + index;

                assert(index < length());
                return chars[index];
            }

            template <unsigned N>
            METHOD Array<BasicString<T>, N> split(T delim = ' ');
            METHOD BasicString<T> rstrip(T white = ' ') const;
            METHOD BasicString<T> lstrip(T white = ' ') const;
            METHOD BasicString<T> strip(T white = ' ') const { return (*this).lstrip(white).rstrip(white); }
            METHOD BasicString<T> slice(int start, size_t len) const;
            METHOD bool startswith(BasicString<T> const& prefix) const;
            METHOD bool endswith(BasicString<T> const& suffix) const;
            METHOD size_t find(BasicString<T> const& str, size_t pos = 0) const;
            METHOD size_t find(const T* s, size_t pos, size_t count) const;
            METHOD size_t find(const T* s, size_t pos = 0) const;
            METHOD size_t find(T s, size_t pos) const;

            template <unsigned N>
            METHOD BasicString<T>& operator =(const T (&str)[N]);
            METHOD BasicString<T>& operator =(BasicString<T> const& str);
            METHOD bool operator ==(BasicString<T> const& other) const;
            METHOD BasicString<T>& operator +=(BasicString<T> const& str);
            METHOD BasicString<T> operator +(BasicString const& str);
            METHOD BasicString<T>& operator *=(unsigned count);
            METHOD BasicString<T> operator *(unsigned count);
            METHOD BasicString<T> operator ()(int start, size_t len) const { return this->slice(start, len); }


            template <typename Func>
            METHOD void each_line(Func cb, T delim = '\n') {
              size_t offset = 0;
              size_t pos = 0;
              BasicString<T> substr = "";

              while ((pos = this->find(delim, offset)) != npos) {
                substr = this->slice(offset, pos - offset);
                if ( cb(substr) != 0 ) {
                  break;
                }
                offset = pos + 1;
              }
            }

            // Static functions.
            template <unsigned N>
            FUNCTION PURE size_t length(const T (&)[N]) { return N-1; }
            FUNCTION PURE size_t length(const T *s);
            FUNCTION PURE int compare(const T *s1, const T *s2);
            FUNCTION PURE int compare(const T *s1, const T *s2, size_t n);
            FUNCTION PURE bool equals(const T *s1, const T *s2);
            FUNCTION T *copy(T *dest, const T *src);
            FUNCTION T *copy(T *dest, const T *src, size_t n);
            FUNCTION T *concat(T *dest, const T *src);
            FUNCTION T *concat(T *dest, const T *src, size_t n);
            FUNCTION __attribute__((format (printf, 1, 2))) BasicString<T> sprintf(const char *format, ...);

        private:
            METHOD void resize(size_t new_size);

            T *chars;
            size_t size;        // Current characters length (including the null character).
            size_t max_size;    // Maximum characters length (including the null character).
            bool needs_dealloc = false;
    };

    using String = BasicString<char>;
    using WideString = BasicString<wchar_t>;

    template <>
    CONSTRUCTOR
    BasicString<char>::BasicString()
    {
        chars = const_cast<char *>("");
        max_size = size = 1;
    }

    template <>
    CONSTRUCTOR
    BasicString<wchar_t>::BasicString()
    {
        chars = const_cast<wchar_t *>(L"");
        max_size = size = 1;
    }


    template <typename T>
    METHOD BasicString<T> BasicString<T>::lstrip(T white) const
    {
        size_t count = 0;

        for ( size_t i = 0; i < length(); i++ )
        {
            if ( chars[i] != white )
                break;

            count++;
        }

        return slice(count, length() - count);
    }

    template <typename T>
    METHOD BasicString<T> BasicString<T>::rstrip(T white) const
    {
        size_t count = 0;

        for ( size_t i = 0; i < length(); i++ )
        {
            if ( chars[length() - 1 - i] != white )
                break;

            count++;
        }

        return slice(0, length() - count);
    }

    template <typename T>
    template <unsigned N>
    METHOD
    Array<BasicString<T>, N> BasicString<T>::split(T delim)
    {
        Array<BasicString<T>, N> array;
        size_t count = 0;
        BasicString<T> trimmed = this->strip(delim);
        int i, pos = 0;

        for ( i = 0; i < trimmed.length(); i++ )
        {
            if ( trimmed[i] != delim )
                continue;

            array[count++] = trimmed.slice(pos, i - pos);

            while ( trimmed[i] == delim && i < trimmed.length() )
                i++;

            pos = i;

            if ( i < trimmed.length() && count == N - 1 ) {
                array[N - 1] = trimmed.slice(pos, trimmed.length() - pos);
                i = pos = trimmed.length();
                count++;
                break;
            }
        }

        if ( count < N && pos < trimmed.length())
            array[count] = trimmed.slice(pos, i - pos);

        return array;
    }

    template <typename T>
    METHOD
    BasicString<T> BasicString<T>::slice(int start, size_t len) const
    {
        if ( start < 0 )
            start = length() + start;

        assert(start >= 0 && start < length());

        if ( start + len >= length() )
            len = length() - start;

        T *substr = new T[len + 1];
        BasicString<T>::copy(substr, chars + start, len);
        substr[len] = '\0';

        return BasicString<T>(substr, len + 1, true);
    }

    template <typename T>
    METHOD
    bool BasicString<T>::startswith(BasicString<T> const& prefix) const
    {
        if ( prefix.length() > this->length() )
            return false;

        return BasicString<T>::compare(chars, prefix.chars, prefix.length());
    }

    template <typename T>
    METHOD
    bool BasicString<T>::endswith(BasicString<T> const& suffix) const
    {
        if ( suffix.length() > this->length() )
            return false;

        return BasicString<T>::compare(chars + length() - suffix.length(), suffix.chars, suffix.length());
    }

    template <typename T>
    METHOD
    size_t BasicString<T>::find(BasicString<T> const& str, size_t pos) const {
      if (pos >= this->length()) {
        return npos;
      }

      if (str.length() > this->length()) {
        return npos;
      }
      //!!!! NAIVE IMPLEMENTATION !!!!\\
      //TODO: Implement Rabin-Karp / KMP algo

      const size_t str_length = str.length();
      const size_t this_idx_max = this->length() - str_length + 1;
      for (size_t this_idx = pos; this_idx < this_idx_max; ++this_idx) {
        size_t str_idx = 0;
        while (str_idx < str_length and str[str_idx] == this->operator[](this_idx + str_idx)) {
          ++str_idx;
        }

        if (str_idx == str_length) {
          return this_idx;
        }
      }

      return npos;

    }


    template <typename T>
    METHOD
    size_t BasicString<T>::find(T c, size_t pos) const {
      if (pos >= this->length()) {
        return npos;
      }

      for (size_t idx = pos; idx < this->length(); ++idx) {
        if (this->operator[](idx) == c) {
          return idx;
        }
      }

      return npos;

    }


    template <typename T>
    METHOD
    size_t BasicString<T>::find(const T* s, size_t pos, size_t count) const {
      if (pos >= this->length()) {
        return npos;
      }

      if (count > this->length()) {
        return npos;
      }

      BasicString<T> str{s, count};
      return this->find(str, pos);

    }

    template <typename T>
    METHOD
    size_t BasicString<T>::find(const T* s, size_t pos) const {
      return this->find(s, pos, length(s));
    }



    template <typename T>
    METHOD
    void BasicString<T>::resize(size_t new_size)
    {
        // Truncating.
        if ( new_size <= max_size )
        {
            max_size = new_size;
            chars[max_size - 1] = 0;

            if ( size > new_size )
                size = new_size;

            return;
        }

        // Expanding.
        T *old_chars = chars;
        chars = new T[new_size];
        BasicString<T>::copy(chars, old_chars);

        if ( needs_dealloc )
            delete[] old_chars;

        needs_dealloc = true;
    }

    // Copy assignment constructors.
    template <typename T>
    template <unsigned N>
    METHOD
    BasicString<T>& BasicString<T>::operator =(const T (&str)[N])
    {
        if ( needs_dealloc )
            delete[] chars;

        chars = const_cast<T *>(str);
        max_size = size = N;

        return *this;
    }

    template <typename T>
    METHOD
    BasicString<T>& BasicString<T>::operator =(BasicString<T> const& str)
    {
        if ( max_size < str.length() + 1 )
        {
            if ( needs_dealloc )
                delete[] chars;

            chars = new T[str.length() + 1];
            max_size = str.length() + 1;
            needs_dealloc = true;
        }

        BasicString<T>::copy(chars, str.pointer());
        size = str.length() + 1;

        return *this;
    }

    template <typename T>
    METHOD
    BasicString<T>& BasicString<T>::operator +=(BasicString<T> const& str)
    {
        size_t total_length = this->length() + str.length();

        if ( max_size < total_length + 1 )
        {
            resize(total_length + 1);
        }

        BasicString<T>::concat(chars, str.pointer());
        size += str.length();

        return *this;
    }

    template <typename T>
    METHOD
    BasicString<T> BasicString<T>::operator +(BasicString<T> const& str)
    {
        String result(this->length() + str.length() + 1);

        result = *this;
        result += str;

        return result;
    }

    template <typename T>
    METHOD
    BasicString<T>& BasicString<T>::operator *=(unsigned count)
    {
        size_t total_length = this->length() * count;

        if ( max_size < total_length + 1 )
        {
            resize(total_length + 1);
        }

        for ( size_t i = 0; i < count; i++ )
            BasicString<T>::concat(chars, chars, this->length());

        size = total_length + 1;
        chars[total_length] = 0;

        return *this;
    }

    template <typename T>
    METHOD
    BasicString<T> BasicString<T>::operator *(unsigned count)
    {
        String result(this->length() * count + 1);

        for ( size_t i = 0; i < count; i++ )
            result += *this;

        return result;
    }

    template <typename T>
    METHOD
    bool BasicString<T>::operator ==(BasicString<T> const& other) const
    {
        return BasicString<T>::equals(chars, other.chars);
    }

    template <>
    METHOD PURE
    size_t BasicString<char>::length(const char *s)
    {
        if ( Options::use_builtins )
           return BUILTIN(strlen)(s);

        return tstrlen(s);
    }

    template <typename T>
    METHOD PURE
    size_t BasicString<T>::length(const T *s)
    {
        return tstrlen(s);
    }

    template <>
    METHOD
    int BasicString<char>::compare(const char *s1, const char *s2)
    {
        if ( Options::use_builtins )
            return BUILTIN(strcmp)(s1, s2);

        return tstrcmp(s1, s2);
    }

    template <typename T>
    METHOD
    int BasicString<T>::compare(const T *s1, const T *s2)
    {
        return tstrcmp(s1, s2);
    }

    template <>
    METHOD
    int BasicString<char>::compare(const char *s1, const char *s2, size_t n)
    {
        if ( Options::use_builtins )
            return BUILTIN(strncmp)(s1, s2, n);

        return tstrncmp(s1, s2, n);
    }

    template <typename T>
    METHOD
    int BasicString<T>::compare(const T *s1, const T *s2, size_t n)
    {
        return tstrcmp(s1, s2, n);
    }

    template <typename T>
    METHOD
    bool BasicString<T>::equals(const T *s1, const T *s2)
    {
        return BasicString<T>::compare(s1, s2) == 0;
    }

    template <>
    METHOD
    char *BasicString<char>::copy(char *dest, const char *src)
    {
        if ( Options::use_builtins )
            return BUILTIN(strcpy)(dest, src);

        return tstrcpy(dest, src);
    }

    template <typename T>
    METHOD
    T *BasicString<T>::copy(T *dest, const T *src)
    {
        return tstrcpy(dest, src);
    }

    template <>
    METHOD
    char *BasicString<char>::copy(char *dest, const char *src, size_t n)
    {
        if ( Options::use_builtins )
           return BUILTIN(strncpy)(dest, src, n);

        return tstrncpy(dest, src, n);
    }

    template <typename T>
    METHOD
    T *BasicString<T>::copy(T *dest, const T *src, size_t n)
    {
        return tstrncpy(dest, src, n);
    }

    template <>
    METHOD
    char *BasicString<char>::concat(char *dest, const char *src)
    {
        if ( Options::use_builtins )
            return BUILTIN(strcat)(dest, src);

        return tstrcat(dest, src);
    }

    template <typename T>
    METHOD
    T *BasicString<T>::concat(T *dest, const T *src)
    {
        return tstrcat(dest, src);
    }

    template <>
    METHOD
    char *BasicString<char>::concat(char *dest, const char *src, size_t n)
    {
        if ( Options::use_builtins )
            return BUILTIN(strncat)(dest, src, n);

        return tstrncat(dest, src, n);
    }

    template <typename T>
    METHOD
    T *BasicString<T>::concat(T *dest, const T *src, size_t n)
    {
        return tstrncat(dest, src, n);
    }

    static constexpr bool isascii(signed char c)
    {
        return ( c >= 0 );
    }

    static constexpr bool isprint(char c)
    {
        return ( c >= ' ' && c <= '~' );
    }

    static constexpr bool iscntrl(char c)
    {
        return isascii(c) && !isprint(c);
    }

    static constexpr bool isdigit(char c)
    {
        return ( c >= '0' && c <= '9' );
    }

    static constexpr bool isxdigit(char c)
    {
        return isdigit(c) || ( c >= 'A' && c <= 'F') || (c >= 'a' && c <= 'f' );
    }

    static constexpr bool isblank(char c)
    {
        return ( c == ' ' || c == '\t' );
    }

    static constexpr bool isspace(char c)
    {
        return ( c == ' ' || c == '\f' || c == '\n' || c == '\r' || c == '\t' || c == '\v' );
    }

    static constexpr bool isgraph(char c)
    {
        return isprint(c) && !isspace(c);
    }

    static constexpr bool islower(char c)
    {
        return ( c >= 'a' && c <= 'z' );
    }

    static constexpr bool isupper(char c)
    {
        return ( c >= 'A' && c <= 'Z' );
    }

    static constexpr bool isalpha(char c)
    {
        return islower(c) || isupper(c);
    }

    static constexpr bool isalnum(char c)
    {
        return isalpha(c) || isdigit(c);
    }

    static constexpr bool ispunct(char c)
    {
        return isgraph(c) && !isalnum(c);
    }

    static constexpr char toupper(char c)
    {
        return islower(c) ? (c - ('a' - 'A')) : c;
    }

    static constexpr char tolower(char c)
    {
        return isupper(c) ? (c + ('a' - 'A')) : c;
    }

    static constexpr char num_to_digit(unsigned long value)
    {
        return ( value < 10 ) ? ( value + '0' ) : ( value - 10 + 'a' );
    }

    static constexpr unsigned long digit_to_num(char digit)
    {
        return ( digit >= 'a' ) ? ( digit - 'a' + 10 ) : ( digit - '0' );
    }

    FUNCTION
    long long int strtoll(const char *nptr, char **endptr, int radix = 10)
    {
        long long int result = 0;
        int sign = 1;
        long long unsigned int pow = 1;
        unsigned nr_digits = 0;

        while ( isspace(*nptr) )
            nptr++;

        if ( *nptr == '+' )
        {
            nptr++;
            sign = 1;
        }
        else if ( *nptr == '-' )
        {
            nptr++;
            sign = -1;
        }

        if ( radix == 16 && nptr[0] == '0' && nptr[1] == 'x' )
            nptr += 2;

        char *dptr = const_cast<char *>(nptr);
        while ( tolower(*dptr) >= num_to_digit(0) && tolower(*dptr) <= num_to_digit(radix - 1) )
        {
            nr_digits++;
            dptr++;
        }

        for ( size_t i = 0; i < nr_digits; i++ )
        {
            result += digit_to_num(tolower(nptr[nr_digits - i - 1])) * pow;
            pow *= radix;
        }

        if ( endptr )
            *endptr = dptr;

        return result * sign;
    }

    FUNCTION
    int atoi(const char *nptr)
    {
        return strtoll(nptr, nullptr);
    }
}

#endif
