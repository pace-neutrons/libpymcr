/* Own copy of MatlabCppSharedLib.hpp with only the functionality we need.
 * This is because these files are not included in the distribution of
 * Matlab on github-hosted action runners for MacOS X and Windows.
 */

// Uncomment to use the official interface
//#include <MatlabCppSharedLib.hpp>

#ifdef _MSC_VER
#define DLL_SYM __declspec(dllexport)
#elif __GNUC__ >= 4
#define DLL_SYM __attribute__((visibility("default")))
#else
#define DLL_SYM
#endif

#ifndef MATLABCPPSHAREDLIB_HPP
#define MATLABCPPSHAREDLIB_HPP
// Ensures we don't conflict with the official interface
#define MATLAB_APPLICATION_HPP
#define MATLAB_LIBRARY_HPP
#define CPPSHAREDLIB_API_HPP
#define CPPSHAREDLIB_API_UTIL_HPP
#define CPPSHAREDLIB_UTIL_HPP
#define CPPSHAREDLIB_FACTORY_HPP
#define CPPSHAREDLIB_FUTURE_HPP
#define EXECUTION_INTERFACE_HPP

#include <vector>
#include <cstdint>
#include <streambuf>
#include <memory>
#include <string>
#include <algorithm>
#include <future>
#include <MatlabDataArray/MDArray.hpp>
#include <MatlabDataArray/detail/HelperFunctions.hpp>

extern "C" DLL_SYM void runtime_create_session(char16_t** options, size_t size);
extern "C" DLL_SYM void runtime_terminate_session();
extern "C" DLL_SYM uint64_t create_mvm_instance(const char16_t* name, bool* errFlag);
extern "C" DLL_SYM void terminate_mvm_instance(const uint64_t mvmHandle);
extern "C" DLL_SYM void wait_for_figures_to_close(const uint64_t mvmHandle);
extern "C" DLL_SYM uintptr_t cppsharedlib_feval_with_completion(const uint64_t matlabHandle,
    const char* function, size_t nlhs, bool scalar, matlab::data::impl::ArrayImpl** args,
    size_t nrhs, void(*success)(void*, size_t, bool, matlab::data::impl::ArrayImpl**),
    void(*exception)(void*, size_t, bool, size_t, const void*), void* p, void* output,
    void* error, void(*write)(void*, const char16_t*, size_t), void(*deleter)(void*));

namespace {
    typedef std::basic_streambuf<char16_t> StreamBuffer;
}

namespace matlab {

  namespace execution {

    void _promise_data(void *p, size_t nlhs, bool straight, matlab::data::impl::ArrayImpl** plhs) {
        if (nlhs == 0 && straight) {
            reinterpret_cast<std::promise<void>*>(p)->set_value();
        }
        else if (nlhs == 1 && straight) {
            matlab::data::Array v = matlab::data::detail::Access::createObj<matlab::data::Array>(plhs[0]);
            reinterpret_cast<std::promise<matlab::data::Array>*>(p)->set_value(v);
        }
        else {
            std::vector<matlab::data::Array> result;
            for (size_t i = 0; i < nlhs; i++) {
                matlab::data::Array v = matlab::data::detail::Access::createObj<matlab::data::Array>(plhs[i]);
                result.push_back(v);
            }
            reinterpret_cast<std::promise<std::vector<matlab::data::Array> >*>(p)->set_value(result);
        }
    }

    void _promise_exception(void *p, size_t nlhs, bool straight, size_t excNum, const void* msg) {
        const char* message = reinterpret_cast<const char*>(msg);
        std::string errmsg;
        switch (excNum) {
            case 0: errmsg = std::string("CANCELLED: ") + message; break;
            case 1: errmsg = std::string("INTERRUPTED: ") + message; break;
            case 2: errmsg = std::string("EXECUTION: ") + message; break;
            case 3: errmsg = std::string("SYNTAX: ") + message; break;
            case 4: errmsg = std::string("OTHER: ") + message; break;
            case 5: errmsg = std::string("STOPPED: ") + message; break;
        }
        auto err = std::make_exception_ptr<std::runtime_error>(std::runtime_error(errmsg));
        if (nlhs == 0 && straight) {
            reinterpret_cast<std::promise<void>*>(p)->set_exception(err);
        } else if (nlhs == 1 && straight) {
            reinterpret_cast<std::promise<matlab::data::Array>*>(p)->set_exception(err);
        } else {
            reinterpret_cast<std::promise<std::vector<matlab::data::Array> >*>(p)->set_exception(err);
        }
    }

    void _write_buffer(void *buffer, const char16_t* stream, size_t n) {
        std::shared_ptr<StreamBuffer>* output = reinterpret_cast<std::shared_ptr<StreamBuffer>*>(buffer);
        output->get()->sputn(stream, n);
    }

    void _delete_buffer(void* impl) {
        delete static_cast<std::shared_ptr<StreamBuffer>*>(impl);
    }

    class ExecutionInterface {
        public:
        uint64_t matlabHandle;

        std::string convertUTF16StringToASCIIString(const std::u16string &str) {
            std::string target(str.size(), 0);
            const char* u16_src = reinterpret_cast<const char*>(str.c_str());
            for(size_t n = 0; n < str.size(); ++n)
               target[n] = u16_src[2*n];
            return target;
        }

        template <
            bool scalar,
            typename T = typename std::conditional<scalar, matlab::data::Array, std::vector<matlab::data::Array>>::type
        > 
        T _feval(const std::u16string &function, const size_t nlhs, const std::vector<matlab::data::Array> &args,
                 const std::shared_ptr<StreamBuffer> &output = std::shared_ptr<StreamBuffer>(),
                 const std::shared_ptr<StreamBuffer> &error = std::shared_ptr<StreamBuffer>()) {
            size_t nrhs = args.size();
            matlab::data::impl::ArrayImpl** argsImpl = new matlab::data::impl::ArrayImpl*[nrhs];
            for (size_t i = 0; i < nrhs; i++) {
                argsImpl[i] = matlab::data::detail::Access::getImpl<matlab::data::impl::ArrayImpl>(args[i]);
            }
            std::promise<T>* p = new std::promise<T>();
            void* output_ = output ? new std::shared_ptr<StreamBuffer>(std::move(output)) : nullptr;
            void* error_ = error ? new std::shared_ptr<StreamBuffer>(std::move(error)) : nullptr;
            std::string utf8functionname = convertUTF16StringToASCIIString(function);
            uintptr_t handle = cppsharedlib_feval_with_completion(matlabHandle, utf8functionname.c_str(), nlhs, scalar,
                argsImpl, nrhs, &_promise_data, &_promise_exception, p, output_, error_, &_write_buffer, &_delete_buffer);
            return p->get_future().get();
        }

        std::vector<matlab::data::Array> feval(const std::u16string &function, const size_t nlhs,
                                               const std::vector<matlab::data::Array> &args,
                                               const std::shared_ptr<StreamBuffer> &output = std::shared_ptr<StreamBuffer>(),
                                               const std::shared_ptr<StreamBuffer> &error = std::shared_ptr<StreamBuffer>()) {
            return _feval<false>(function, nlhs, args, output, error);
        }
        matlab::data::Array feval(const std::u16string &function, const std::vector<matlab::data::Array> &args,
                                  const std::shared_ptr<StreamBuffer> &output = std::shared_ptr<StreamBuffer>(),
                                  const std::shared_ptr<StreamBuffer> &error = std::shared_ptr<StreamBuffer>()) {
            return _feval<true>(function, 1, args, output, error);
        }
        matlab::data::Array feval(const std::u16string &function, const matlab::data::Array &arg,
                                  const std::shared_ptr<StreamBuffer> &output = std::shared_ptr<StreamBuffer>(),
                                  const std::shared_ptr<StreamBuffer> &error = std::shared_ptr<StreamBuffer>()) {
            std::vector<matlab::data::Array> args = {arg};
            return _feval<true>(function, 1, args, output, error);
        }       
        ExecutionInterface(uint64_t handle) : matlabHandle(handle) {}
        ~ExecutionInterface() {
            matlabHandle = 0;
        }
    };
  } // namespace execution

  namespace cpplib {

    enum class MATLABApplicationMode {
        OUT_OF_PROCESS = 0,
        IN_PROCESS = 1
    };

    void _init(const std::vector<std::u16string>& options) {
        std::vector<char16_t*> options_v(options.size());
        std::transform(options.begin(), options.end(), options_v.begin(), 
                       [](const std::u16string& option) { return const_cast<char16_t*>(option.c_str());  });
        runtime_create_session(options_v.data(), options_v.size());
    }

    class MATLABApplication {
        public:
        MATLABApplicationMode _mode;
        std::vector<std::u16string> _options;
        MATLABApplication(const MATLABApplicationMode mode, const std::vector<std::u16string>& options)
            : _mode(mode), _options(options) {}
        ~MATLABApplication() {
            runtime_terminate_session();
        }
    };

    class MATLABLibrary : public matlab::execution::ExecutionInterface {
        public:
        std::shared_ptr<MATLABApplication> appPtr_;
        MATLABLibrary(std::shared_ptr<MATLABApplication> application, uint64_t handle)
            : matlab::execution::ExecutionInterface(handle), appPtr_(application) {}
        ~MATLABLibrary() {
            if (matlabHandle != 0) {
                terminate_mvm_instance(matlabHandle);
                matlabHandle = 0;
            }
        }
        void waitForFiguresToClose() {
            if (matlabHandle != 0) {
                wait_for_figures_to_close(matlabHandle);
            }
        }
    };

    inline std::shared_ptr<MATLABApplication> initMATLABApplication(const MATLABApplicationMode mode, const std::vector<std::u16string>& options)
    {
        if (mode == MATLABApplicationMode::IN_PROCESS) {
            _init(options);
        } else {
            std::vector<std::u16string> newOptions = {u"-outproc"};
            std::copy(options.begin(), options.end(), std::back_inserter(newOptions));
            _init(newOptions);
        }
        return std::shared_ptr<MATLABApplication>(new MATLABApplication(mode, options));
    }

    inline std::unique_ptr<MATLABLibrary> initMATLABLibrary(std::shared_ptr<MATLABApplication> application, const std::u16string& ctffilename) {
        // Note that ctffilename *must* be an *absolute* path
        bool errFlag = false;
        uint64_t handle = create_mvm_instance(ctffilename.c_str(), &errFlag);
        if (errFlag) {
            throw std::runtime_error("Failed to initialize MATLABlibrary");
        }
        return std::unique_ptr<MATLABLibrary>(new MATLABLibrary(application, handle));
    }

  } // namespace cpplib
} // namespace matlab


#endif // MATLABCPPSHAREDLIB_HPP
