/* Own copy of MatlabDataArray.hpp with only the functionality we need.
 * This enables compiling on github runners without loading Matlab
 */

// Uncomment to use the official interface
//#include <MatlabDataArray.hpp>

//#ifndef MATLAB_DATA_ARRAY_HPP
//#define MATLAB_DATA_ARRAY_HPP

// Uncomment to use the official interface
//#include <MatlabDataArray/MDArray.hpp>
//#include <MatlabDataArray/TypedArray.hpp>
//#include <MatlabDataArray/ArrayFactory.hpp>

// Definitions of <MatlabDataArray/MDArray.hpp> {
#ifndef MDA_ARRAY_HPP_
#define MDA_ARRAY_HPP_
#define MATLAB_DATA_ARRAY_DEFS_H
#define MATLAB_DATA_ARRAY_TYPE_HPP
#define MATLAB_DATA_ARRAY_DIMENSIONS_HPP
#define MATLAB_DATA_MEMORY_LAYOUT_HPP
#define MATLAB_DATA_OPTIONAL_HPP_
#define MATLAB_DATA_STRING_HPP
#define MATLAB_DATA_GET_ARRAY_TYPE_HPP_
#define MATLAB_EXCEPTION_HPP_
#define MATLAB_EXTDATA_ARRAY_ELEMENT_REF_HPP
#define MATLAB_EXTDATA_DEFS_HPP_
#define MATLAB_EXTDATA_HELPER_FUNCTIONS_HPP
#define MATLAB_EXTDATA_EXCEPTION_HELPER_FUNCTIONS_HPP
#define MATLAB_EXTDATA_REFERENCE_HELPERS_HPP
#define MATLAB_EXTDATA_PUBLISH_UTIL_HPP
#define MATLAB_DATA_ENUMERATION_HPP
#define STRING_HELPERS_HPP_

#ifndef _HAS_CHAR16_T_LANGUAGE_SUPPORT
  #define _HAS_CHAR16_T_LANGUAGE_SUPPORT 0
#endif

#if (defined(__cplusplus) && (__cplusplus >= 201103L)) || _HAS_CHAR16_T_LANGUAGE_SUPPORT
  #define U16_STRING_LITERAL_PREFIX u
  typedef char16_t CHAR16_T;
#else
  #define U16_STRING_LITERAL_PREFIX L
  typedef wchar_t CHAR16_T;
  #if !defined(_CHAR16T)
    #define _CHAR16T
    typedef wchar_t char16_t;
  #endif
#endif

#include <complex>
#include <cstdint>
#include <vector>
#include <exception>
//#include <functional>
#include <algorithm>
#include <memory>

#ifdef _MSC_VER
# define DLLSYM __declspec(dllimport)
#elif __GNUC__ >= 4
# define DLLSYM __attribute__ ((visibility("default")))
#else
# define DLLSYM
#endif

#if defined(_MSC_FULL_VER) && _MSC_FULL_VER < 190023026
  #define MW_NOEXCEPT
#else
  #define MW_NOEXCEPT noexcept
#endif

#define InvalidArrayTypeException std::runtime_error
#define NonAsciiCharInInputDataException std::runtime_error
#define FeatureNotSupportedException std::runtime_error
#define InvalidDimensionsInSparseArrayException std::runtime_error

// Definitions of functions in libMatlabDataArray
extern "C" DLLSYM void* get_function_ptr(int fcn); 
namespace matlab { namespace data { namespace detail { 
    enum class FunctionType;
    template <class T> T resolveFunction(FunctionType fcn) {
        void* ptr = get_function_ptr(static_cast<int>(fcn));
        if (ptr == nullptr) { throw std::runtime_error("Could not load matlab data function"); }
        return (T)ptr;
    }
} } }

extern "C" DLLSYM void* array_create_empty();
extern "C" DLLSYM void array_destroy_impl(void* ptr);
extern "C" DLLSYM int array_get_type(void* impl, int* type);
extern "C" DLLSYM int array_get_memory_layout(void* impl, int* layout);
extern "C" DLLSYM void array_get_dimensions(void* impl, size_t* ndims, size_t** dims);
extern "C" DLLSYM size_t array_get_number_of_elements(void* impl);
extern "C" DLLSYM bool array_is_empty(void* impl);
extern "C" DLLSYM bool array_unshare(void* impl, bool unique, void** newImpl);
extern "C" DLLSYM int array_create_reference(void* pImpl, size_t idx, void** impl);
extern "C" DLLSYM void array_reference_shared_copy(void* impl, void** Aimpl);
extern "C" DLLSYM int array_release_buffer(void* pImpl, bool unique, void** buffer, void(**)(void*));
extern "C" DLLSYM int array_factory_create(void** impl);
extern "C" DLLSYM void array_factory_destroy_impl(void* impl);
extern "C" DLLSYM void add_name(void* impl, const char* name, size_t nameLen);
extern "C" DLLSYM void* create_names(size_t num);
extern "C" DLLSYM int create_array_with_dims(void* impl, int type, size_t* dims, size_t nD, void** arrayImpl);
extern "C" DLLSYM int create_array_with_dims_and_data(void* impl, int type, size_t* dims, size_t nD, const void* const dataStart, size_t numEl, void** aImpl);
extern "C" DLLSYM int create_buffer(void* impl, void** buffer, void(**)(void*), int dataType, size_t numElements);
extern "C" DLLSYM int create_array_from_buffer_v2(void* impl, int tp, size_t* dims, size_t nD, void* buf, void (*)(void*), void**, int layout);
extern "C" DLLSYM int create_char_array_from_string(void* impl, const char* data, size_t strlen, void** arrayImpl);
extern "C" DLLSYM int create_scalar_array(void* impl, int arrayType, const void* data, void** arrayImpl);
extern "C" DLLSYM int create_struct_array(void* pImpl, size_t* dims, size_t nD, void* names, void** impl);
extern "C" DLLSYM int char_array_get_ascii(void*, const char16_t**, size_t*);
extern "C" DLLSYM int enum_array_get_value(void*, const char**, size_t*);
extern "C" DLLSYM void forward_iterator_destroy_impl(void*);
extern "C" DLLSYM bool forward_iterator_equal(void* lhs, void* rhs);
extern "C" DLLSYM void forward_iterator_get_ref(void* impl, void** refcnt);
extern "C" DLLSYM void forward_iterator_plus_plus(void*);
extern "C" DLLSYM void names_destroy_impl(void* impl);
extern "C" DLLSYM void* field_id_create_impl(const char* str, size_t sz);
extern "C" DLLSYM void field_id_destroy_impl(void* ptr);
extern "C" DLLSYM void* field_id_get_string(void* ref, const char** str, size_t* len);
extern "C" DLLSYM int reference_add_index(void* pImpl, size_t idx);
extern "C" DLLSYM int reference_add_string_index(void* impl, const char* str, size_t idx);
extern "C" DLLSYM void reference_destroy_impl(void* ptr);
extern "C" DLLSYM int reference_set_char16_string(void* impl, const char16_t* str, size_t sz);
extern "C" DLLSYM int reference_set_string(void* impl, const char* str, size_t sz);
extern "C" DLLSYM int reference_set_reference_value(void* imp, void* rhs);
extern "C" DLLSYM int reference_set_char16_string(void* impl, const char16_t* elem, size_t sz);
extern "C" DLLSYM int reference_set_missing_char16_string(void* str);
extern "C" DLLSYM int reference_get_container_type(void* impl, int* type);
//extern "C" DLLSYM bool reference_matlab_string_equal_string(void*, const char*, size_t idx);
//extern "C" DLLSYM bool reference_matlab_string_equal_string16(void* impl, const char16_t*, size_t idx);
extern "C" DLLSYM int string_get_value(void* impl, const char16_t**, size_t*);
extern "C" DLLSYM void* struct_array_begin_id(void* impl);
extern "C" DLLSYM void* struct_array_end_id(void* impl);
extern "C" DLLSYM int typed_reference_set_pod_value(void* impl, int type, void* rhs);
extern "C" DLLSYM int typed_reference_set_complex_value(void* impl, int type, void* real, void* imag);
extern "C" DLLSYM int typed_reference_get_pod_value(void* impl, void** value);
extern "C" DLLSYM int typed_reference_get_complex_value(void* impl, void** real, void** imag);
extern "C" DLLSYM int typed_array_reference_validate_type(void* impl, int type, bool flag);
extern "C" DLLSYM int typed_array_is_valid_conversion(int type_lhs, int type_rhs, bool* result);
extern "C" DLLSYM void* typed_array_begin_v2(void*, bool);
extern "C" DLLSYM void* typed_array_end_v2(void*, bool);
extern "C" DLLSYM void typed_iterator_destroy_impl(void* impl);
extern "C" DLLSYM bool typed_iterator_equal(void*, void*);
extern "C" DLLSYM void typed_iterator_get_proxy(void*, void**);
extern "C" DLLSYM void typed_iterator_plus_plus(void*);
extern "C" DLLSYM int struct_get_field_copy(void* impl, const char* str, size_t sz, void** Aimpl);
extern "C" DLLSYM int string_get_value(void* impl, const char16_t** str, size_t* strlen);

namespace matlab {
  class Exception {
  public:
    virtual ~Exception() MW_NOEXCEPT {}
    virtual const char* what() const MW_NOEXCEPT = 0;
  };
  namespace data {
    namespace impl {
        class ArrayImpl;
    }
    class Enumeration {};
    template <typename T> struct is_complex : std::false_type {};
    template <typename T> struct is_complex<std::complex<T>> : std::true_type {};
    template <typename T> struct is_complex<std::complex<T> const> : std::true_type {};
    using buffer_deleter_t = void(*)(void*);
    template <typename T> using buffer_ptr_t = std::unique_ptr<T[], buffer_deleter_t>;
    using void_buffer_ptr_t = std::unique_ptr<void, buffer_deleter_t>;
    using ArrayDimensions = std::vector<size_t>;
    using String = std::basic_string<char16_t>;
    template<typename T> class optional {
        public:
        typedef T value_type;
        bool fMissing;
        T fData;
        optional() MW_NOEXCEPT { fMissing = true; }
        optional(T&& value) MW_NOEXCEPT : fData(std::move(value)) { fMissing = false; }
        const T& operator*() const {
            if (fMissing) { throw std::runtime_error("Missing string value"); } return const_cast<T&>(fData); }
        T& operator*() {
            if (fMissing) { throw std::runtime_error("Missing string value"); } return fData; }
        operator T() const {
            if (fMissing) { throw std::runtime_error("Missing string value"); } return fData; }
        explicit operator bool() const MW_NOEXCEPT { return !fMissing; }
    };
    using MATLABString = optional<String>;
    class Array;
    class Struct;
    enum class DataOrder : int { COLUMN_MAJOR, ROW_MAJOR };
    enum class MemoryLayout : int { COLUMN_MAJOR = 0, ROW_MAJOR, UNKNOWN };
    enum class ArrayType : int {
        LOGICAL, CHAR, MATLAB_STRING, DOUBLE, SINGLE, INT8, UINT8, INT16, UINT16, INT32, UINT32, INT64, UINT64,
        COMPLEX_DOUBLE, COMPLEX_SINGLE, COMPLEX_INT8, COMPLEX_UINT8, COMPLEX_INT16, COMPLEX_UINT16, COMPLEX_INT32,
        COMPLEX_UINT32, COMPLEX_INT64, COMPLEX_UINT64, CELL, STRUCT, OBJECT, VALUE_OBJECT, HANDLE_OBJECT_REF,
        ENUM, SPARSE_LOGICAL, SPARSE_DOUBLE, SPARSE_COMPLEX_DOUBLE, UNKNOWN,
    };
    struct GetCellType { static const ArrayType type = ArrayType::CELL; };
    struct GetStringType { static const ArrayType type = ArrayType::UNKNOWN; };
    template<typename T> struct GetArrayType;
    template<> struct GetArrayType<bool> { static const ArrayType type = ArrayType::LOGICAL; };
    template<> struct GetArrayType<CHAR16_T> { static const ArrayType type = ArrayType::CHAR; };
    template<> struct GetArrayType<MATLABString> { static const ArrayType type = ArrayType::MATLAB_STRING; };
    template<> struct GetArrayType<double> { static const ArrayType type = ArrayType::DOUBLE; };
    template<> struct GetArrayType<float> { static const ArrayType type = ArrayType::SINGLE; };
    template<> struct GetArrayType<int8_t> { static const ArrayType type = ArrayType::INT8; };
    template<> struct GetArrayType<int16_t> { static const ArrayType type = ArrayType::INT16; };
    template<> struct GetArrayType<int32_t> { static const ArrayType type = ArrayType::INT32; };
    template<> struct GetArrayType<int64_t> { static const ArrayType type = ArrayType::INT64; };
    template<> struct GetArrayType<uint8_t> { static const ArrayType type = ArrayType::UINT8; };
    template<> struct GetArrayType<uint16_t> { static const ArrayType type = ArrayType::UINT16; };
    template<> struct GetArrayType<uint32_t> { static const ArrayType type = ArrayType::UINT32; };
    template<> struct GetArrayType<uint64_t> { static const ArrayType type = ArrayType::UINT64; };
#if !defined(__linux__) && !defined(_WIN32)
    template<> struct GetArrayType<unsigned long> { static const ArrayType type = ArrayType::UINT64; };
#endif
    template<> struct GetArrayType<std::complex<int8_t>> { static const ArrayType type = ArrayType::COMPLEX_INT8; };
    template<> struct GetArrayType<std::complex<int16_t>> { static const ArrayType type = ArrayType::COMPLEX_INT16; };
    template<> struct GetArrayType<std::complex<int32_t>> { static const ArrayType type = ArrayType::COMPLEX_INT32; };
    template<> struct GetArrayType<std::complex<int64_t>> { static const ArrayType type = ArrayType::COMPLEX_INT64; };
    template<> struct GetArrayType<std::complex<uint8_t>> { static const ArrayType type = ArrayType::COMPLEX_UINT8; };
    template<> struct GetArrayType<std::complex<uint16_t>> { static const ArrayType type = ArrayType::COMPLEX_UINT16; };
    template<> struct GetArrayType<std::complex<uint32_t>> { static const ArrayType type = ArrayType::COMPLEX_UINT32; };
    template<> struct GetArrayType<std::complex<uint64_t>> { static const ArrayType type = ArrayType::COMPLEX_UINT64; };
    template<> struct GetArrayType<std::complex<double>> { static const ArrayType type = ArrayType::COMPLEX_DOUBLE; };
    template<> struct GetArrayType<std::complex<float>> { static const ArrayType type = ArrayType::COMPLEX_SINGLE; };
    template<> struct GetArrayType<Array> { static const ArrayType type = ArrayType::CELL; };
    template<> struct GetArrayType<Struct> { static const ArrayType type = ArrayType::STRUCT; };
    namespace detail {
      class ReferenceImpl;
      class Access {
        public:
        template<typename ImplT, typename ObjT> static ImplT* getImpl(const ObjT& a) {
            return static_cast<ImplT *>(a.pImpl.get()); }
        template <typename Obj, typename Impl> static Obj createObj(Impl* impl) {
            return Obj(impl); }
        template <typename Obj, typename Impl> static Obj createObj(std::shared_ptr<Impl> impl) {
            return Obj(impl); }
        template <typename Obj, typename Impl, typename T> static Obj createObj(Impl* impl, T arg1) {
            return Obj(impl, arg1); }
      };
      inline bool isAscii7(const String &str) {
          return (std::find_if(str.begin(), str.end(), [](CHAR16_T ch) {return (ch & 0xFF80) != 0; }) == str.end());
      }
      inline bool isAscii7(const std::string &str) {
          return (std::find_if(str.begin(), str.end(), [](char ch) {return (ch & 0x80) != 0; }) == str.end());
      }
      inline void throwIfError(int type) {
        switch (type) {
            case 0: break;
            case 1:  throw std::runtime_error("Array index is invalid");
            case 2:  throw std::runtime_error("String index must be last");
            case 3:  throw std::runtime_error("String index not valid for this array");
            case 5:  throw std::runtime_error("Not enough indices provided");
            case 6:  throw std::runtime_error("Too many indices provided");
            case 7:  throw std::runtime_error("Can only provide one string index");
            case 8:  throw std::runtime_error("Data type mismatch");
            case 9:  throw std::runtime_error("Array type mismatch");
            case 10: throw std::runtime_error("Field does not exist in this struct");
            case 13: throw std::runtime_error("Must specify a class name");
            case 14: throw std::runtime_error("Wrong number of enums supplied");
            case 16: throw std::runtime_error("Can't create a std::string from this data");
            case 18: throw std::runtime_error("Duplicate field names not allowed");
            case 19: throw std::runtime_error("Not enough memory available to support the request");
            case 20: throw std::runtime_error("Matlab system error");
            case 21: throw std::runtime_error("Can't index into an empty array");
            case 23: throw std::runtime_error("Memory layout mismatch");
            case 24: throw std::runtime_error("Memory layout mismatch");
            case 25: throw std::runtime_error("Memory layout mismatch");
            case 26: throw std::runtime_error("Number of elements exceeds max");
            default: throw std::runtime_error("Unknown Matlab Error");
        }
      }
      template <typename U>
      inline typename std::enable_if<std::is_arithmetic<U>::value, U&>::type getElement(std::shared_ptr<ReferenceImpl> impl) {
          void* value = nullptr;
          throwIfError(typed_reference_get_pod_value(impl.get(), &value));
          return *(static_cast<U*>(value));
      }
      template <typename U>
          inline typename std::enable_if<is_complex<U>::value, U>::type getElement(std::shared_ptr<ReferenceImpl> impl) {
          void *real = nullptr, *imag = nullptr;
          throwIfError(typed_reference_get_complex_value(impl.get(), &real, &imag));
          typename U::value_type r = *(static_cast<typename U::value_type*>(real));
          typename U::value_type i = *(static_cast<typename U::value_type*>(imag));
          return U(r,i);
      }
      template <typename U> inline typename std::enable_if<std::is_same<MATLABString, U>::value, MATLABString>::type getElement(std::shared_ptr<ReferenceImpl> impl) {
        const char16_t* str = nullptr;
        size_t strlen = 0;
        throwIfError(string_get_value(impl.get(), &str, &strlen));
        if (str != nullptr) {
            return MATLABString(String(str, strlen));
        }
        return MATLABString();
      }
      template <typename U> inline typename std::enable_if<std::is_same<Array, U>::value, Array>::type getElement(std::shared_ptr<ReferenceImpl> impl) {
          void* arr_impl = nullptr;
          array_reference_shared_copy(impl.get(), &arr_impl);
          return Access::createObj<U>(static_cast<impl::ArrayImpl*>(arr_impl));
      }
      template <typename U> inline typename std::enable_if<std::is_same<Struct, U>::value, Struct>::type getElement(std::shared_ptr<ReferenceImpl> impl) {
          return Access::createObj<U>(impl);
      }
      template <typename U> inline typename std::enable_if<std::is_arithmetic<U>::value>::type setElement(ReferenceImpl* impl, U rhs, int type) {
        throwIfError(typed_reference_set_pod_value(impl, type, &rhs)); }
      template <typename U> inline typename std::enable_if<is_complex<U>::value>::type setElement(ReferenceImpl* impl, U rhs, int type) {
        typename U::value_type real = rhs.real();
        typename U::value_type imag = rhs.imag();
        throwIfError(typed_reference_set_complex_value(impl, type, &real, &imag));
      }
      template <typename U> inline typename std::enable_if<std::is_same<String, U>::value>::type setElement(ReferenceImpl* impl, U rhs, int) {
        throwIfError(reference_set_char16_string(impl, rhs.c_str(), rhs.size())); }
      template <typename U> inline typename std::enable_if<std::is_same<std::string, U>::value>::type setElement(ReferenceImpl* impl, U rhs, int) {
        throwIfError(reference_set_string(impl, rhs.c_str(), rhs.size())); }
      template <typename U> inline typename std::enable_if<std::is_base_of<Array, U>::value>::type setElement(ReferenceImpl* impl, U rhs, int) {
        throwIfError(reference_set_reference_value(impl, Access::getImpl<matlab::data::impl::ArrayImpl>(rhs))); }
      template <typename U> inline typename std::enable_if<std::is_same<MATLABString, U>::value>::type setElement(ReferenceImpl* impl, U rhs, int) {
        if (rhs) {
            String elem = *rhs;
            throwIfError(reference_set_char16_string(impl, elem.c_str(), elem.size()));
        } else {
            throwIfError(reference_set_missing_char16_string(impl));
        }
      }
      template <typename U>
      inline typename std::enable_if<!std::is_same<Enumeration, U>::value &&
                                     !std::is_same<MATLABString, U>::value, std::string>::type getString(ReferenceImpl* impl) {
        throw std::runtime_error("Invalid data type");
      }
      template <typename U>
      inline typename std::enable_if<std::is_same<Enumeration, U>::value, std::string>::type getString(ReferenceImpl* impl) {
          const char* str = nullptr;
          size_t strlen = 0;
          throwIfError(enum_array_get_value(impl, &str, &strlen));
          return std::string(str, strlen);
      }

      template <typename U>
      inline typename std::enable_if<std::is_same<MATLABString, U>::value, std::string>::type getString(ReferenceImpl* impl) {
          const char16_t* str = nullptr;
          size_t strlen = 0;
          bool missing = false;
          throwIfError(string_get_value(impl, &str, &strlen));
          if (str == nullptr) {
              throw std::runtime_error("Missing string");
          }
          String temp(str, strlen);
          if (!isAscii7(temp)) {
              throw std::runtime_error("Input data can only contain ASCII characters");
          }
        return std::string(temp.begin(), temp.end());
      }
      template <typename T> inline T castToType(void* value, ArrayType type) {
        switch (type) {
            case ArrayType::LOGICAL: return static_cast<T>(*static_cast<bool*>(value));
            case ArrayType::DOUBLE: return static_cast<T>(*static_cast<double*>(value));
            case ArrayType::SINGLE: return static_cast<T>(*static_cast<float*>(value));
            case ArrayType::INT64: return static_cast<T>(*static_cast<int64_t*>(value));
            case ArrayType::INT32: return static_cast<T>(*static_cast<int32_t*>(value));
            case ArrayType::INT16: return static_cast<T>(*static_cast<int16_t*>(value));
            case ArrayType::INT8: return static_cast<T>(*static_cast<int8_t*>(value));
            case ArrayType::UINT64: return static_cast<T>(*static_cast<uint64_t*>(value));
            case ArrayType::UINT32: return static_cast<T>(*static_cast<uint32_t*>(value));
            case ArrayType::UINT16: return static_cast<T>(*static_cast<uint16_t*>(value));
            case ArrayType::UINT8: return static_cast<T>(*static_cast<uint8_t*>(value));
            default:
                throw std::runtime_error("Can't convert this element");
        }
        return T();
      }
      template <typename T, bool is_const_ref> typename std::enable_if<std::is_arithmetic<T>::value, T>::type castTo(std::shared_ptr<ReferenceImpl> impl) {
        void* value = nullptr;
        throwIfError(typed_reference_get_pod_value(impl.get(), &value));
        int type;
        throwIfError(reference_get_container_type(impl.get(), &type));
        return castToType<T>(value, static_cast<ArrayType>(type));
      }
      template <typename T, bool is_const_ref> typename std::enable_if<std::is_base_of<Array, T>::value, T>::type castTo(std::shared_ptr<ReferenceImpl> impl) {
        throwIfError(typed_array_reference_validate_type(impl.get(), static_cast<int>(T::type), false));
        void* arr_impl = nullptr;
        array_reference_shared_copy(impl.get(), &arr_impl);
        return Access::createObj<T>(static_cast<impl::ArrayImpl*>(arr_impl));
      }
    }
    template <bool is_const_ref>
    class ArrayElementRef {
      public:
        ArrayElementRef<is_const_ref> operator[](size_t idx) {
            detail::throwIfError(reference_add_index(pImpl.get(), idx));
            return ArrayElementRef<is_const_ref>(pImpl);
        }
        ArrayElementRef<is_const_ref> operator[](std::string idx) {
            detail::throwIfError(reference_add_string_index(pImpl.get(), idx.c_str(), idx.size()));
            return ArrayElementRef<is_const_ref>(pImpl);
        }
        template <typename T> operator T() const { return detail::castTo<T, is_const_ref>(pImpl); }
        template <typename T> ArrayElementRef<is_const_ref>& operator= (T rhs) {
            static_assert(!is_const_ref, "Can't modify a const reference");
            typedef typename std::conditional<std::is_same<std::string, T>::value || std::is_same<String, T>::value, GetStringType, GetArrayType<T>>::type GetNonArrayType;
            typedef typename std::conditional<std::is_base_of<Array, T>::value, GetCellType, GetNonArrayType>::type GetType;
            detail::setElement(pImpl.get(), std::move(rhs), static_cast<int>(GetType::type));
            return *this;
        }
      private:
        std::shared_ptr<detail::ReferenceImpl> pImpl;
        ArrayElementRef(ArrayElementRef<is_const_ref>&& rhs) MW_NOEXCEPT : pImpl(std::move(rhs.pImpl)) {}
        ArrayElementRef(detail::ReferenceImpl* impl) MW_NOEXCEPT :
            pImpl(std::shared_ptr<detail::ReferenceImpl> (impl, [](detail::ReferenceImpl* ptr) { reference_destroy_impl(ptr); })) {}
        ArrayElementRef(std::shared_ptr<detail::ReferenceImpl> impl) MW_NOEXCEPT : pImpl(impl) {}
        friend class detail::Access;
        friend class Array;
        ArrayElementRef() = delete;
        ArrayElementRef(const ArrayElementRef<is_const_ref>& rhs) = delete;
        ArrayElementRef<is_const_ref>& operator=(const ArrayElementRef<is_const_ref>& rhs) = delete;
        ArrayElementRef<is_const_ref>& operator=(ArrayElementRef<is_const_ref>&& rhs) = delete;
    };
    class Array {
      public:
        static const ArrayType type = ArrayType::UNKNOWN;
        Array() MW_NOEXCEPT {
            pImpl = std::shared_ptr<impl::ArrayImpl>(reinterpret_cast<impl::ArrayImpl*>(array_create_empty()),
            [](impl::ArrayImpl* ptr) { array_destroy_impl(ptr); });
        }
        virtual ~Array() MW_NOEXCEPT {}
        Array(Array&& rhs) MW_NOEXCEPT : pImpl(std::move(rhs.pImpl)) {}
        Array& operator=(Array&& rhs) MW_NOEXCEPT {
            pImpl = std::move(rhs.pImpl);
            return *this;
        }
        Array& operator=(const Array& rhs) MW_NOEXCEPT {
            pImpl = rhs.pImpl;
            return *this;
        }
        Array(const Array& rhs) MW_NOEXCEPT : pImpl(rhs.pImpl) {}
        ArrayType getType() const {
            int type;
            detail::throwIfError(array_get_type(pImpl.get(), &type));
            return static_cast<ArrayType>(type);
        }
        MemoryLayout getMemoryLayout() const {
            int layout;
            detail::throwIfError(array_get_memory_layout(pImpl.get(), &layout));
            return static_cast<MemoryLayout>(layout);
        }
        ArrayDimensions getDimensions() const MW_NOEXCEPT {
            size_t numDims = 0;
            size_t* dims = nullptr;
            array_get_dimensions(pImpl.get(), &numDims, &dims);
            return ArrayDimensions(dims, dims + numDims);
        }
        size_t getNumberOfElements() const MW_NOEXCEPT {
            return array_get_number_of_elements(pImpl.get()); }
        bool isEmpty() const MW_NOEXCEPT {
            return array_is_empty(pImpl.get()); }
        ArrayElementRef<false> operator[](size_t idx) {
            void* newImpl = nullptr;
            if (array_unshare(pImpl.get(), (pImpl.use_count() == 1), &newImpl)) {
                pImpl.reset(static_cast<impl::ArrayImpl*>(newImpl), [](impl::ArrayImpl* ptr) {
                    array_destroy_impl(ptr);
                });
            }
            void* impl = nullptr;
            detail::throwIfError(array_create_reference(pImpl.get(), idx, &impl));
            return detail::Access::createObj<ArrayElementRef<false>>(static_cast<detail::ReferenceImpl*>(impl));
        }
        ArrayElementRef<true> operator[](size_t idx) const {
            void* impl = nullptr;
            detail::throwIfError(array_create_reference(pImpl.get(), idx, &impl));
            return detail::Access::createObj<ArrayElementRef<true>>(static_cast<detail::ReferenceImpl*>(impl));
        }
        protected:
        Array(impl::ArrayImpl* impl) MW_NOEXCEPT
        : pImpl(std::shared_ptr<impl::ArrayImpl>(impl, [](impl::ArrayImpl* ptr) {
            array_destroy_impl(ptr);
        })) {}
        std::shared_ptr<impl::ArrayImpl> pImpl;
        friend class detail::Access;
    };
  } // namespace matlab::data
} // namespace matlab

#endif // MDA_ARRAY_HPP_ }

// Definitions of <MatlabDataArray/TypedArray.hpp> {
#ifndef TYPED_ARRAY_HPP_
#define TYPED_ARRAY_HPP_

#define MATLAB_DATA_STRUCT_HPP
#define MATLAB_EXTDATA_ARRAY_ELEMENT_TYPED_REF_HPP
#define MATLAB_EXTDATA_ARRAY_REFERENCE_HOLDER_HPP
#define MATLAB_EXTDATA_ARRAY_REF_EXT_HPP
#define MATLAB_EXTDATA_MATLABSTRING_REF_EXT_HPP
#define MATLAB_EXTDATA_REFERENCE_HPP
#define MATLAB_DATA_TYPED_ITERATOR_HPP

#include <algorithm>

namespace matlab {
  namespace data {
    namespace detail {
        class ReferenceImpl;
        class IteratorImpl;
    }
    class ReferenceHolder {
      protected:
        std::shared_ptr<detail::ReferenceImpl> pImpl;
        ReferenceHolder(detail::ReferenceImpl* impl) MW_NOEXCEPT : 
            pImpl(std::shared_ptr<detail::ReferenceImpl>(impl, [](detail::ReferenceImpl* ptr) { reference_destroy_impl(ptr); })) {}
        ReferenceHolder(std::shared_ptr<detail::ReferenceImpl> impl) MW_NOEXCEPT : pImpl(impl) {}
        ReferenceHolder(ReferenceHolder&& rhs) MW_NOEXCEPT : pImpl(std::move(rhs.pImpl)) {}
        ReferenceHolder& operator= (ReferenceHolder&& rhs) MW_NOEXCEPT {
            pImpl = std::move(rhs.pImpl);
            return *this;
        }
        ReferenceHolder(const ReferenceHolder& rhs) MW_NOEXCEPT : pImpl(rhs.pImpl) {}
        ReferenceHolder& operator= (const ReferenceHolder& rhs) MW_NOEXCEPT {
            pImpl = rhs.pImpl;
            return *this;
        }
    };
    class ArrayReferenceExt : public ReferenceHolder {
      public:
        ArrayReferenceExt(detail::ReferenceImpl* impl) MW_NOEXCEPT : ReferenceHolder(impl) {}
        ArrayReferenceExt(std::shared_ptr<detail::ReferenceImpl> impl) MW_NOEXCEPT : ReferenceHolder(impl) {}
        ArrayReferenceExt(ArrayReferenceExt&& rhs) MW_NOEXCEPT : ReferenceHolder(std::move(rhs)) {}
    };
    class MATLABStringReferenceExt : public ReferenceHolder {
      public:
        operator String() const {
            MATLABString opt = detail::getElement<MATLABString>(pImpl);
            if (!opt) {
                throw std::runtime_error("Missing string value");
            }
            return *opt;
        }
    };
    template <bool is_const_ref> class IndexableArrayRef : public ArrayReferenceExt { };
    template <typename T> struct GetReferenceExt { using type = ReferenceHolder; };
    template <> struct GetReferenceExt<Array> { using type = IndexableArrayRef<false>; };
    template <> struct GetReferenceExt<MATLABString> { using type = MATLABStringReferenceExt; };
    template <typename T> class Reference : public GetReferenceExt<T>::type { };
    using buffer_deleter_t = void(*)(void*);
    template <typename T> using buffer_ptr_t = std::unique_ptr<T[], buffer_deleter_t>;
    template <typename T> class TypedArray;
    template <typename T> struct GetElementReferenceType      { using type = ReferenceHolder; };
    template <> struct GetElementReferenceType<Array>         { using type = ArrayReferenceExt; };
    template <> struct GetElementReferenceType<MATLABString > { using type = MATLABStringReferenceExt; };
    template <typename T, bool is_const_ref>
    class ArrayElementTypedRef : public GetElementReferenceType<T>::type {
      public:
        using Parent = typename GetElementReferenceType<T>::type;
        using Parent::pImpl;
        ArrayElementTypedRef<T, is_const_ref> operator[](size_t idx) {
            detail::throwIfError(reference_add_index(pImpl.get(), idx));
            return ArrayElementTypedRef<T, is_const_ref>(pImpl);
        }
        ArrayElementTypedRef<Array, is_const_ref> operator[](std::string idx) {
            detail::throwIfError(reference_add_string_index(pImpl.get(), idx.c_str(), idx.size()));
            return detail::Access::createObj<ArrayElementTypedRef<Array, is_const_ref>>(pImpl);
        }
        ArrayElementTypedRef(ArrayElementTypedRef<T, is_const_ref>&& rhs) MW_NOEXCEPT : Parent(std::move(rhs.pImpl)) {}
        ArrayElementTypedRef(detail::ReferenceImpl* impl) MW_NOEXCEPT : Parent(impl) {}
        ArrayElementTypedRef(std::shared_ptr<detail::ReferenceImpl> impl) MW_NOEXCEPT : Parent(impl) {}
        typedef typename std::conditional<std::is_arithmetic<T>::value && !is_const_ref, T&, T>::type reference;
        operator reference() const { return detail::getElement<T>(pImpl); }
        operator std::string() const { return detail::getString<T>(pImpl.get()); }
        ArrayElementTypedRef<T, is_const_ref>& operator= (T rhs) {
            static_assert(!is_const_ref, "Can't modify a reference to const data");
            detail::setElement(pImpl.get(), std::move(rhs), static_cast<int>(GetArrayType<T>::type));
            return *this;
        }
/*      ArrayElementTypedRef<T, is_const_ref>& operator= (std::string rhs) {
            static_assert(!is_const_ref, "Can't modify a reference to const data");
            detail::setString<T>(pImpl.get(), std::move(rhs));
            return *this;
        }*/
/*      template <typename U> operator Reference<U>() const {
            static_assert(!is_const_ref, "Can only get a reference to a non const ref");
            detail::validateReference<T, U>(pImpl.get(), true);
            return detail::Access::createObj<Reference<U>>(pImpl);
        }*/
        template <typename U, typename Type = T, typename = typename std::enable_if<std::is_base_of<Array, Type>::value>::type> operator U() const {
            static_assert(std::is_base_of<Array, U>::value, "Can only be used to cast to an Array");
            return detail::castTo<U, is_const_ref>(pImpl);
        }
    };
/*  template<bool is_const_ref> bool operator ==(ArrayElementTypedRef<MATLABString, is_const_ref> const& lhs, std::string const& rhs) MW_NOEXCEPT {
        return reference_matlab_string_equal_string(detail::Access::getImpl<detail::ReferenceImpl>(lhs), rhs.c_str(), rhs.size());
    }

    template<bool is_const_ref> bool operator ==(std::string const& lhs, ArrayElementTypedRef<MATLABString, is_const_ref> const& rhs) MW_NOEXCEPT {
        return reference_matlab_string_equal_string(detail::Access::getImpl<detail::ReferenceImpl>(rhs), lhs.c_str(), lhs.size());
    }
    template<bool is_const_ref> bool operator ==(ArrayElementTypedRef<MATLABString, is_const_ref> const& lhs, String const& rhs) MW_NOEXCEPT {
        return reference_matlab_string_equal_string16(detail::Access::getImpl<detail::ReferenceImpl>(lhs), rhs.c_str(), rhs.size());
    }
    template<bool is_const_ref> bool operator ==(String const& lhs, ArrayElementTypedRef<MATLABString, is_const_ref> const& rhs) MW_NOEXCEPT {
        return reference_matlab_string_equal_string16(detail::Access::getImpl<detail::ReferenceImpl>(rhs), lhs.c_str(), lhs.size());
    }*/
    template <typename T> struct GetConstRefType    { using type = T; };
    template <> struct GetConstRefType<Enumeration> { using type = std::string; };
    template<typename T>
    class TypedIterator {
      public:
        typedef typename std::remove_const<T>::type impl_type;
        typedef typename GetConstRefType<impl_type>::type const_reference;
        typedef typename std::conditional<std::is_const<T>::value, const_reference, Reference<impl_type>>::type ref_type;
        typedef typename std::conditional<std::is_arithmetic<T>::value || is_complex<T>::value, T&, ref_type>::type reference;
        template <typename U = T>
        typename std::enable_if<std::is_arithmetic<U>::value || is_complex<U>::value, reference>::type operator*() const MW_NOEXCEPT {
            return *fRef;
        }
        template <typename U = T>
        typename std::enable_if<!std::is_arithmetic<U>::value && !is_complex<U>::value, reference>::type operator*() const MW_NOEXCEPT {
            return getRef<T>();
        }
        template <typename U = T>
        typename std::enable_if<std::is_arithmetic<U>::value || is_complex<U>::value, bool>::type operator!=(const TypedIterator<T> &rhs) const MW_NOEXCEPT {
            return fRef != rhs.fRef;
        }
        template <typename U = T>
        typename std::enable_if<!std::is_arithmetic<U>::value && !is_complex<U>::value, bool>::type operator!=(const TypedIterator<T> &rhs) const MW_NOEXCEPT {
            return !typed_iterator_equal(pImpl.get(), rhs.pImpl.get());
        }
        template <typename U = T>
        typename std::enable_if<std::is_arithmetic<U>::value || is_complex<U>::value, TypedIterator<T>&>::type operator++() MW_NOEXCEPT {
            ++fRef;
            return *this;
        }
        template <typename U = T>
        typename std::enable_if<!std::is_arithmetic<U>::value && !is_complex<U>::value, TypedIterator<T>&>::type operator++() MW_NOEXCEPT {
            typed_iterator_plus_plus(pImpl.get());
            return *this;
        }
/*      template <typename U = T>
        typename std::enable_if<std::is_arithmetic<U>::value || is_complex<U>::value, TypedIterator<T>>::type operator++(int) MW_NOEXCEPT {
            TypedIterator<T> temp(*this);
            ++fRef;
            return temp;
        }
        template <typename U = T>
        typename std::enable_if<!std::is_arithmetic<U>::value && !is_complex<U>::value, TypedIterator<T>>::type operator++(int) MW_NOEXCEPT {
            TypedIterator<T> temp(*this);
            typed_iterator_plus_plus(pImpl.get());
            return temp;
        }*/
      private:
        typedef typename std::conditional<std::is_arithmetic<T>::value || is_complex<T>::value, impl_type*, ref_type>::type TempRef;
        friend class detail::Access;
        std::shared_ptr<detail::IteratorImpl> pImpl;
        mutable TempRef fRef;
        TypedIterator(detail::IteratorImpl *impl) MW_NOEXCEPT
            : fRef(createTempRef<impl_type, std::is_const<T>::value>(std::shared_ptr<detail::IteratorImpl>
                (impl, [](detail::IteratorImpl* ptr) { typed_iterator_destroy_impl(ptr); }))) {}
        template <typename U, bool is_const> typename std::enable_if<!std::is_arithmetic<U>::value && !is_complex<U>::value && !std::is_same<Struct, U>::value && is_const, ref_type>::type
        createTempRef(std::shared_ptr<detail::IteratorImpl> impl) MW_NOEXCEPT {
            pImpl = std::move(impl);
            return ref_type();
        }
        template <typename U, bool is_const> typename std::enable_if<!std::is_arithmetic<U>::value && !is_complex<U>::value && std::is_same<Struct, U>::value && is_const, ref_type>::type
        createTempRef(std::shared_ptr<detail::IteratorImpl> impl) MW_NOEXCEPT {
            pImpl = std::move(impl);
            return detail::Access::createObj<U>(std::shared_ptr<detail::ReferenceImpl>());
        }
        template <typename U> typename std::enable_if<!std::is_arithmetic<U>::value && !is_complex<U>::value, reference>::type getRef() const MW_NOEXCEPT {
            void* value = nullptr;
            typed_iterator_get_proxy(pImpl.get(), &value);
            auto ref = detail::Access::createObj<Reference<impl_type>>(static_cast<detail::ReferenceImpl*>(value));
            return ref;
        }
    };
    template<typename T>
    class TypedArray : public Array {
      public:
        using iterator = TypedIterator<T>;
        using const_iterator = TypedIterator<typename std::add_const<T>::type>;
        using data_type = T;
        static const ArrayType type = GetArrayType<typename std::remove_const<T>::type>::type;
        virtual ~TypedArray() MW_NOEXCEPT {}
        TypedArray<T>& operator=(TypedArray<T>&& rhs) MW_NOEXCEPT {
            Array::operator=(std::move(rhs));
            return *this;
        }
        TypedArray(TypedArray<T>&& rhs) MW_NOEXCEPT : Array(std::move(rhs)) {}
        TypedArray(const TypedArray<T> &rhs) MW_NOEXCEPT : Array(rhs) {}
        TypedArray(Array&& rhs) : Array(std::move(rhs)) {
            bool result;
            detail::throwIfError(typed_array_is_valid_conversion(static_cast<int>(GetArrayType<T>::type), static_cast<int>(getType()), &result));
            if (!result) {
                rhs = std::move(*this);
                throw std::runtime_error("Can't convert the Array to this TypedArray");
            }
        }
        TypedArray<T>& operator=(Array&& rhs) {
            bool result;
            detail::throwIfError(typed_array_is_valid_conversion(static_cast<int>(GetArrayType<T>::type), static_cast<int>(rhs.getType()), &result));
            if (!result) {
                throw InvalidArrayTypeException("Can't convert the Array to this TypedArray"); }
            Array::operator=(std::move(rhs));
            return *this;
        }
        TypedArray(const Array& rhs) : Array(rhs) {
            bool result;
            detail::throwIfError(typed_array_is_valid_conversion(static_cast<int>(GetArrayType<T>::type), static_cast<int>(rhs.getType()), &result));
            if (!result) {
                throw InvalidArrayTypeException("Can't convert the Array to this TypedArray"); }
        }
        TypedArray<T>& operator=(const Array& rhs) {
            bool result;
            detail::throwIfError(typed_array_is_valid_conversion(static_cast<int>(GetArrayType<T>::type), static_cast<int>(rhs.getType()), &result));
            if (!result) {
                throw InvalidArrayTypeException("Can't convert the Array to this TypedArray"); }
            Array::operator=(rhs);
            return *this;
        }
        TypedArray<T>& operator=(const TypedArray<T>& rhs) MW_NOEXCEPT {
            Array::operator=(rhs);
            return *this;
        }
        iterator begin() MW_NOEXCEPT {
            void* newImpl = nullptr;
            if (array_unshare(pImpl.get(), (pImpl.use_count() == 1), &newImpl)) {
                pImpl.reset(static_cast<impl::ArrayImpl*>(newImpl), [](impl::ArrayImpl* ptr) { array_destroy_impl(ptr); });
            }
            return detail::Access::createObj<iterator>(static_cast<detail::IteratorImpl*>(typed_array_begin_v2(pImpl.get(), !std::is_const<T>::value)));
        }
        const_iterator begin() const MW_NOEXCEPT {
            return detail::Access::createObj<const_iterator>(static_cast<detail::IteratorImpl*>(typed_array_begin_v2(pImpl.get(), false)));
        }
        iterator end() MW_NOEXCEPT {
            void* newImpl = nullptr;
            if (array_unshare(pImpl.get(), (pImpl.use_count() == 1), &newImpl)) {
                pImpl.reset(static_cast<impl::ArrayImpl*>(newImpl), [](impl::ArrayImpl* ptr) { array_destroy_impl(ptr); });
            }
            return detail::Access::createObj<iterator>(typed_array_end_v2(pImpl.get(), !std::is_const<T>::value));
        }
        const_iterator end() const MW_NOEXCEPT {
            return detail::Access::createObj<const_iterator>(typed_array_end_v2(pImpl.get(), false));
        }
        ArrayElementTypedRef<T, std::is_const<T>::value> operator[](size_t idx) {
            void* newImpl = nullptr;
            if (array_unshare(pImpl.get(), (pImpl.use_count() == 1), &newImpl)) {
                pImpl.reset(static_cast<impl::ArrayImpl*>(newImpl), [](impl::ArrayImpl* ptr) { array_destroy_impl(ptr); }); }
            void* impl = nullptr;
            detail::throwIfError(array_create_reference(pImpl.get(), idx, &impl));
            return detail::Access::createObj<ArrayElementTypedRef<T, std::is_const<T>::value>>(static_cast<detail::ReferenceImpl*>(impl));
        }
        ArrayElementTypedRef<T, true> operator[](size_t idx) const {
            void* impl = nullptr;
            detail::throwIfError(array_create_reference(pImpl.get(), idx, &impl));
            return detail::Access::createObj<ArrayElementTypedRef<T, true>>(static_cast<detail::ReferenceImpl*>(impl));
        }
        buffer_ptr_t<T> release() {
            void* buffer = nullptr;
            buffer_deleter_t deleter = nullptr;
            detail::throwIfError(array_release_buffer(pImpl.get(), (pImpl.use_count() == 1), &buffer, &deleter));
            return buffer_ptr_t<T>(static_cast<T*>(buffer), deleter);
        }
      protected:
        friend class detail::Access;
        TypedArray(impl::ArrayImpl* impl) MW_NOEXCEPT : Array(impl) {}
        TypedArray() = delete;
    };
    class Struct {
      public:
        Array operator[](std::string idx) const {
            void* arrayImpl = nullptr;
            detail::throwIfError(struct_get_field_copy(pImpl.get(), idx.c_str(), idx.size(), &arrayImpl));
            return detail::Access::createObj<Array>(static_cast<impl::ArrayImpl*>(arrayImpl));
        }
      private:
        Struct() = delete;
        Struct(std::shared_ptr<detail::ReferenceImpl> impl) MW_NOEXCEPT : pImpl(impl) {}
        std::shared_ptr<detail::ReferenceImpl> pImpl;
        friend class detail::Access;
    };
    using CellArray = TypedArray<Array>;
    using StringArray = TypedArray<MATLABString>;

  } // namespace matlab::data
} // namespace matlab

#endif // TYPED_ARRAY_HPP_ }

// Definitions of <MatlabDataArray/ArrayFactory.hpp> {
#ifndef MATLAB_DATA_ARRAY_FACTORY_HPP_
#define MATLAB_DATA_ARRAY_FACTORY_HPP_

#define CHAR_ARRAY_HPP_
#define MATLAB_DATA_STRUCT_ARRAY_HPP
#define MATLAB_DATA_FIELDIDENTIFIER_HPP
#define MATLAB_DATA_GET_RETURN_TYPE_HPP_

namespace matlab {
  namespace data {
    namespace impl {
        class ArrayFactoryImpl;
    }
    namespace detail {
        class NameListImpl;
        class RefCounted;
        class ForwardIteratorImpl;
    }
    class MATLABFieldIdentifier {
      public:
        MATLABFieldIdentifier(std::string str) MW_NOEXCEPT {
            pImpl = std::shared_ptr<detail::RefCounted> (static_cast<detail::RefCounted*>(field_id_create_impl(str.c_str(), str.size())), 
                    [](detail::RefCounted* ptr) { field_id_destroy_impl(ptr); });
        }
        operator std::string() const MW_NOEXCEPT {
            const char* str = nullptr;
            size_t len = 0;
            field_id_get_string(pImpl.get(), &str, &len);
            return std::string(str, len);
        }
      private:
        std::shared_ptr<detail::RefCounted> pImpl;
    };
    template<typename T> struct GetReturnType { typedef T type; };
    template<> struct GetReturnType<String> { typedef MATLABString type; };
    template<> struct GetReturnType<std::string> { typedef MATLABString type; };
    template<> struct GetReturnType<const char*> { typedef MATLABString type; };
    template<> struct GetReturnType<const char16_t*> { typedef MATLABString type; };
    template<> struct GetReturnType<char> { typedef CHAR16_T type; };
    namespace detail {
      template <typename T, typename ItType> 
      typename std::enable_if<(matlab::data::is_complex<T>::value || std::is_arithmetic<T>::value) && !std::is_same<T, bool>::value, TypedArray<typename GetReturnType<T>::type>>::type
      createArrayWithIterator(matlab::data::impl::ArrayFactoryImpl* impl, ArrayDimensions dims, ItType begin, ItType end) {
        void* aImpl = nullptr;
        throwIfError(create_array_with_dims_and_data(impl, static_cast<int>(GetArrayType<T>::type), &dims[0], dims.size(), &(*(begin)), (end - begin), &aImpl));
        return Access::createObj<TypedArray<typename GetReturnType<T>::type>>(static_cast<matlab::data::impl::ArrayImpl*>(aImpl));
      }
    }
    template<template<class> class IteratorType, class ElementType> class Range {
      public:
        Range(IteratorType<ElementType> begin, IteratorType<ElementType> end) MW_NOEXCEPT : fBegin(std::move(begin)), fEnd(std::move(end)) {}
        Range(Range&& rhs) MW_NOEXCEPT : fBegin(std::move(rhs.fBegin)), fEnd(std::move(rhs.fEnd)) {}
        IteratorType<ElementType>& begin() MW_NOEXCEPT { return fBegin; }
        IteratorType<ElementType>& end() MW_NOEXCEPT { return fEnd; }
      private:
        IteratorType<ElementType> fBegin;
        IteratorType<ElementType> fEnd;
        Range& operator=(Range const& rhs) = delete;
        Range(const Range &rhs) = delete;
    };
    template<typename T> class ForwardIterator {
      public:
        std::shared_ptr<detail::ForwardIteratorImpl> pImpl;
        mutable std::remove_const<T> fRef;
        size_t fIndex;
        ForwardIterator(detail::ForwardIteratorImpl *impl) 
            : pImpl(std::shared_ptr<detail::ForwardIteratorImpl>(impl, [](detail::ForwardIteratorImpl* ptr) { forward_iterator_destroy_impl(ptr); })), fIndex(0) {}
        ForwardIterator<T>& operator++() MW_NOEXCEPT {
            ++fIndex;
            forward_iterator_plus_plus(pImpl.get());
            return *this;
        }
        bool operator!=(const ForwardIterator<T> &rhs) const MW_NOEXCEPT {
            return !forward_iterator_equal(pImpl.get(), rhs.pImpl.get());
        }
        T operator*() const MW_NOEXCEPT {
            void* value = nullptr;
            forward_iterator_get_ref(pImpl.get(), &value);
            return detail::Access::createObj<T>(static_cast<detail::RefCounted*>(value), fIndex);
        }
    };
    class CharArray : public TypedArray<CHAR16_T> {
      public:
        std::string toAscii() const
        {
            const char16_t* strVal = nullptr;
            size_t strLen = 0;
            detail::throwIfError(char_array_get_ascii(detail::Access::getImpl<impl::ArrayImpl>(*this), &strVal, &strLen));
            return std::string(strVal, strVal+strLen);
        }
        CharArray(impl::ArrayImpl* impl) MW_NOEXCEPT : TypedArray<CHAR16_T>(impl) {}
    };
    class StructArray : public TypedArray<Struct> {
      public:
        StructArray(Array&& rhs) : TypedArray<Struct>(std::move(rhs)) {}
        StructArray(const Array& rhs) : TypedArray<Struct>(rhs) {}
        Range<ForwardIterator, MATLABFieldIdentifier const> getFieldNames() const MW_NOEXCEPT {
            return Range<ForwardIterator, MATLABFieldIdentifier const>(
                detail::Access::createObj<ForwardIterator<MATLABFieldIdentifier const>>(
                    static_cast<detail::ForwardIteratorImpl*>(struct_array_begin_id(detail::Access::getImpl<impl::ArrayImpl>(*this)))),
                detail::Access::createObj<ForwardIterator<MATLABFieldIdentifier const>>(
                    static_cast<detail::ForwardIteratorImpl*>(struct_array_end_id(detail::Access::getImpl<impl::ArrayImpl>(*this)))));
        }
     private:
        friend class detail::Access;
    };
    class ArrayFactory {
      public:
        ArrayFactory() {
            void* impl = nullptr;
            detail::throwIfError(array_factory_create(&impl));
            pImpl = std::shared_ptr<impl::ArrayFactoryImpl>(static_cast<impl::ArrayFactoryImpl*>(impl), 
                [](impl::ArrayFactoryImpl* ptr) { array_factory_destroy_impl(ptr); });
        }
        ~ArrayFactory() MW_NOEXCEPT {}
        template <typename T> TypedArray<T> createArray(ArrayDimensions dims) {
            void* impl = nullptr;
            detail::throwIfError(create_array_with_dims(pImpl.get(), static_cast<int>(GetArrayType<T>::type), &dims[0], dims.size(), &impl));
            return detail::Access::createObj<TypedArray<T>>(static_cast<impl::ArrayImpl*>(impl));
        }
        template <typename ItType, typename T = typename std::remove_cv<typename std::iterator_traits<ItType>::value_type>::type>
        TypedArray<typename GetReturnType<T>::type> createArray(ArrayDimensions dims, ItType begin, ItType end) {
            return detail::createArrayWithIterator<T>(pImpl.get(), std::move(dims), begin, end);
        }
        template <typename T> typename std::enable_if<std::is_arithmetic<T>::value, TypedArray<T>>::type createScalar(const T val) {
            void* impl = nullptr;
            detail::throwIfError(create_scalar_array(pImpl.get(), static_cast<int>(GetArrayType<T>::type), &val, &impl));
            return detail::Access::createObj<TypedArray<T>>(static_cast<impl::ArrayImpl*>(impl));
        }
        template <typename T> typename std::enable_if<is_complex<T>::value, TypedArray<T>>::type createScalar(const T val) {
            auto arr = createArray<T>({1, 1});
            arr[0] = val;
            return arr;
        }
        CellArray createCellArray(ArrayDimensions dims) {
            return createArray<Array>(std::move(dims));
        }        
        StructArray createStructArray(ArrayDimensions dims, std::vector<std::string> fieldNames) {
            NameList field_names(static_cast<detail::NameListImpl*>(create_names(fieldNames.size())));
            for (const auto& field : fieldNames) {
                add_name(field_names.getImpl(), field.c_str(), field.size());
            }
            void* impl = nullptr;
            detail::throwIfError(create_struct_array(pImpl.get(), &dims[0], dims.size(), field_names.getImpl(), &impl));
            return detail::Access::createObj<StructArray>(static_cast<impl::ArrayImpl*>(impl));
        }
        CharArray createCharArray(std::string str) {
            if (!detail::isAscii7(str)) {
                throw std::runtime_error(std::string("Input data can only contain ASCII characters"));
            }
            void* impl = nullptr;
            detail::throwIfError(create_char_array_from_string(pImpl.get(), str.c_str(), str.size(), &impl));
            return detail::Access::createObj<CharArray>(static_cast<impl::ArrayImpl*>(impl));
        }
        template <typename T> buffer_ptr_t<T> createBuffer(size_t numberOfElements) {
            void* buffer = nullptr;
            buffer_deleter_t deleter = nullptr;
            detail::throwIfError(create_buffer(pImpl.get(), &buffer, &deleter, static_cast<int>(GetArrayType<T>::type), numberOfElements));
            return buffer_ptr_t<T>(static_cast<T*>(buffer), deleter);
        }
        template <typename T>
        TypedArray<T> createArrayFromBuffer(ArrayDimensions dims, buffer_ptr_t<T> buffer, MemoryLayout memoryLayout = MemoryLayout::COLUMN_MAJOR) {
            buffer_deleter_t deleter = buffer.get_deleter();
            void* impl = nullptr;
            detail::throwIfError(create_array_from_buffer_v2(pImpl.get(), static_cast<int>(GetArrayType<T>::type), &dims[0],
                dims.size(), buffer.release(), deleter, &impl, static_cast<int>(memoryLayout)));
            return detail::Access::createObj<TypedArray<T>>(static_cast<impl::ArrayImpl*>(impl));
        }
      protected:
        std::shared_ptr<impl::ArrayFactoryImpl> pImpl;
      private:
        class NameList {
          public:
            NameList(detail::NameListImpl* impl)
            : pImpl(std::shared_ptr<detail::NameListImpl>(impl, [](detail::NameListImpl* ptr) { names_destroy_impl(ptr); })) { }
            detail::NameListImpl* getImpl() const { return pImpl.get(); }
          private:
            std::shared_ptr<detail::NameListImpl> pImpl;
        };
    };

  } // namespace matlab::data
} // namespace matlab

#endif // MATLAB_DATA_ARRAY_FACTORY_HPP_ }
