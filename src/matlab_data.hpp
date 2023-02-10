/* Own copy of MatlabDataArray.hpp with only the functionality we need.
 * This enables compiling on github runners without loading Matlab
 */

// Uncomment to use the official interface
//#include <MatlabDataArray.hpp>

//#ifndef MATLAB_DATA_ARRAY_HPP
//#define MATLAB_DATA_ARRAY_HPP

// Uncomment to use the official interface
#include <MatlabDataArray/MDArray.hpp>
//#include <MatlabDataArray/TypedArray.hpp>
//#include <MatlabDataArray/ArrayFactory.hpp>
//#include <MatlabDataArray/CharArray.hpp>
//#include <MatlabDataArray/StructArray.hpp>

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
extern "C" DLLSYM int reference_add_index(void* pImpl, size_t idx);
extern "C" DLLSYM int reference_add_string_index(void* impl, const char* str, size_t idx);
extern "C" DLLSYM void reference_destroy_impl(void* ptr);
extern "C" DLLSYM int typed_reference_set_pod_value(void* impl, int type, void* rhs);
extern "C" DLLSYM int typed_reference_set_complex_value(void* impl, int type, void* real, void* imag);
extern "C" DLLSYM int reference_set_char16_string(void* impl, const char16_t* str, size_t sz);
extern "C" DLLSYM int reference_set_string(void* impl, const char* str, size_t sz);
extern "C" DLLSYM int reference_set_reference_value(void* imp, void* rhs);
extern "C" DLLSYM int reference_set_char16_string(void* impl, const char16_t* elem, size_t sz);
extern "C" DLLSYM int reference_set_missing_char16_string(void*);
extern "C" DLLSYM int typed_reference_get_pod_value(void* impl, void** value);
extern "C" DLLSYM int reference_get_container_type(void* impl, int* type);
extern "C" DLLSYM int typed_array_reference_validate_type(void* impl, int type, bool flag);
extern "C" DLLSYM void array_reference_shared_copy(void* impl, void** Aimpl);
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
            return static_cast<ImplT *>(a.pImpl.get());
        }
        template <typename Obj, typename Impl> static Obj createObj(Impl* impl) {
            return Obj(impl);
        }
      };
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
      template <typename U> inline typename std::enable_if<std::is_same<MATLABString, U>::value, MATLABString>::type getElement(std::shared_ptr<ReferenceImpl> impl) {
        const char16_t* str = nullptr;
        size_t strlen = 0;
        throwIfError(string_get_value(impl.get(), &str, &strlen));
        if (str != nullptr) {
            return MATLABString(String(str, strlen));
        }
        return MATLABString();
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

/*
// Definitions of <MatlabDataArray/TypedArray.hpp> {
#ifndef TYPED_ARRAY_HPP_
#define TYPED_ARRAY_HPP_

#define MATLAB_DATA_STRUCT_HPP
#define MATLAB_DATA_TYPED_ITERATOR_HPP


namespace matlab {
  namespace data {

    template<typename T>
    class TypedArray : public Array {
      public:
        using iterator = TypedIterator<T>;
        using const_iterator = TypedIterator<typename std::add_const<T>::type>;
        using reference = typename iterator::reference;
        using const_reference = typename const_iterator::reference;
        using data_type = T;
        static const ArrayType type = GetArrayType<typename std::remove_const<T>::type>::type;
        virtual ~TypedArray() MW_NOEXCEPT {}
        TypedArray<T>& operator=(TypedArray<T>&& rhs) MW_NOEXCEPT {
            Array::operator=(std::move(rhs));
            return *this;
        }
    };
    class Struct {
      public:
        typedef TypedIterator<Array const> const_iterator;
        Array operator[](std::string idx) const {
            void* arrayImpl = nullptr;
            detail::throwIfError(struct_get_field_copy(pImpl.get(), idx.c_str(), idx.size(), &arrayImpl));
            return detail::Access::createObj<Array>(static_cast<impl::ArrayImpl*>(arrayImpl));
        }
    };
    using CellArray = TypedArray<Array>;
    using StringArray = TypedArray<MATLABString>;

  } // namespace matlab::data
} // namespace matlab

#endif // TYPED_ARRAY_HPP_ }
*/
