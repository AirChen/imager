// Copyright (c) 2024 Minmin Gong
//

#include "PythonSystem.hpp"
#include <vector>
#include <iostream>

namespace AIHoloImager
{
    PyObjectPtr MakePyObjectPtr(PyObject* p)
    {
        return PyObjectPtr(p);
    }

    class PythonSystem::Impl
    {
    public:
        explicit Impl(const std::filesystem::path& exe_dir)
        {
            // /usr/lib/python3.10 runtime dir: /usr/lib/x86_64-linux-gnu exe dir "/root/ws/imager/build/bin"

            std::cout << AIHI_PY_STDLIB_DIR << " runtime dir: " << AIHI_PY_RUNTIME_LIB_DIR << " exe dir " << exe_dir << std::endl;
            std::filesystem::path stdlib_dir = std::filesystem::path(AIHI_PY_STDLIB_DIR);
            std::vector<std::wstring> paths;
            paths.push_back(std::filesystem::path(AIHI_PY_STDLIB_DIR).lexically_normal().wstring());
            paths.push_back((stdlib_dir / "config-3.10-x86_64-linux-gnu").lexically_normal().wstring());
            paths.push_back((std::filesystem::path(AIHI_PY_RUNTIME_LIB_DIR)).lexically_normal().wstring());
            paths.push_back(exe_dir.lexically_normal().wstring());
            paths.push_back((exe_dir / "Python/Lib/site-packages").lexically_normal().wstring());
            paths.push_back((exe_dir / "InstantMesh").lexically_normal().wstring());

            PyPreConfig pre_config;
            PyPreConfig_InitIsolatedConfig(&pre_config);

            pre_config.utf8_mode = 1;

            PyStatus status = Py_PreInitialize(&pre_config);
            if (PyStatus_Exception(status))
            {
                Py_ExitStatusException(status);
            }

            PyConfig config;
            PyConfig_InitIsolatedConfig(&config);

            status = PyConfig_SetString(&config, &config.program_name, L"AIHoloImager");
            if (PyStatus_Exception(status))
            {
                PyConfig_Clear(&config);
                Py_ExitStatusException(status);
            }

            config.module_search_paths_set = 1;
            uint32_t t = 0;
            for (const auto& path : paths)
            {
                status = PyWideStringList_Append(&config.module_search_paths, path.c_str());
                if (PyStatus_Exception(status))
                {
                    PyConfig_Clear(&config);
                    Py_ExitStatusException(status);
                } else {
                    std::cout << "with no exception: " << t << "\n";
                }
                t++;
            }

            status = Py_InitializeFromConfig(&config);
            if (PyStatus_Exception(status))
            {
                PyConfig_Clear(&config);
                Py_ExitStatusException(status);
            }

            PyConfig_Clear(&config);

            PyObject *sys_path = PySys_GetObject("path");
            PyObject_Print(sys_path, stdout, 0);
        }

        ~Impl()
        {
            Py_Finalize();
        }
    };

    void PythonSystem::Test() {
        auto mask_generator_module = Import("MaskGenerator");
        if (!mask_generator_module) {
            std::cerr << "Failed to import MaskGenerator module\n";
            PyErr_Print();
            return;
        }

        // Debug: List all attributes of the MaskGenerator module
        std::cout << "Listing attributes of MaskGenerator module:\n";
        PyObject* module_dict = PyModule_GetDict(mask_generator_module.get());
        if (module_dict) {
            PyObject* keys = PyDict_Keys(module_dict);
            if (keys) {
                Py_ssize_t size = PyList_Size(keys);
                for (Py_ssize_t i = 0; i < size; i++) {
                    PyObject* key = PyList_GetItem(keys, i);
                    if (key && PyUnicode_Check(key)) {
                        const char* attr_name = PyUnicode_AsUTF8(key);
                        std::cout << "  Attribute: " << attr_name << "\n";
                    }
                }
                Py_DecRef(keys);
            }
        }

        auto mask_generator_class = GetAttr(*mask_generator_module, "MaskGenerator");
        if (!mask_generator_class) {
            std::cerr << "Failed to get MaskGenerator attribute from MaskGenerator module\n";
            PyErr_Print();
            
            // Additional debug: Check if the module has __file__ attribute
            PyObject* file_attr = PyObject_GetAttrString(mask_generator_module.get(), "__file__");
            if (file_attr) {
                const char* file_path = PyUnicode_AsUTF8(file_attr);
                std::cout << "MaskGenerator module file path: " << (file_path ? file_path : "unknown") << "\n";
                Py_DecRef(file_attr);
            }
            return;
        }

        auto mask_generator = CallObject(*mask_generator_class);
        if (!mask_generator) {
            std::cerr << "Failed to create MaskGenerator instance\n";
            PyErr_Print();
            return;
        }

        auto mask_generator_gen_method = GetAttr(*mask_generator, "Gen");
        if (!mask_generator_gen_method) {
            std::cerr << "Failed to get Gen attribute from MaskGenerator instance\n";
            PyErr_Print();
        }

        auto pil_module = Import("PIL");
        auto image_class = GetAttr(*pil_module, "Image");
        auto image_frombuffer_method = GetAttr(*image_class, "frombuffer");
    }

    PythonSystem::PythonSystem(const std::filesystem::path& exe_dir) : impl_(std::make_unique<Impl>(exe_dir))
    {
    }

    PythonSystem::~PythonSystem() noexcept = default;

    PythonSystem::PythonSystem(PythonSystem&& other) noexcept = default;
    PythonSystem& PythonSystem::operator=(PythonSystem&& other) noexcept = default;

    PyObjectPtr PythonSystem::Import(const char* name)
    {
        return MakePyObjectPtr(PyImport_ImportModule(name));
    }

    PyObjectPtr PythonSystem::GetAttr(PyObject& module, const char* name)
    {
        PyObject_Print(&module, stdout, 0);
        std::cout << " attr name: " << name << std::endl;
        return MakePyObjectPtr(PyObject_GetAttrString(&module, name));
    }

    PyObjectPtr PythonSystem::CallObject(PyObject& object)
    {
        return MakePyObjectPtr(PyObject_CallObject(&object, nullptr));
    }

    PyObjectPtr PythonSystem::CallObject(PyObject& object, PyObject& args)
    {
        return MakePyObjectPtr(PyObject_CallObject(&object, &args));
    }

    PyObjectPtr PythonSystem::MakeObject(long value)
    {
        return MakePyObjectPtr(PyLong_FromLong(value));
    }

    PyObjectPtr PythonSystem::MakeObject(std::wstring_view str)
    {
        return MakePyObjectPtr(PyUnicode_FromWideChar(str.data(), str.size()));
    }

    PyObjectPtr PythonSystem::MakeObject(std::span<const std::byte> mem)
    {
        return MakePyObjectPtr(PyBytes_FromStringAndSize(reinterpret_cast<const char*>(mem.data()), mem.size()));
    }

    PyObjectPtr PythonSystem::MakeTuple(uint32_t size)
    {
        return MakePyObjectPtr(PyTuple_New(size));
    }

    void PythonSystem::SetTupleItem(PyObject& tuple, uint32_t index, PyObject& item)
    {
        PyTuple_SetItem(&tuple, index, &item);
    }

    void PythonSystem::SetTupleItem(PyObject& tuple, uint32_t index, PyObjectPtr item)
    {
        PyTuple_SetItem(&tuple, index, item.release());
    }

    template <>
    long PythonSystem::Cast<long>(PyObject& object)
    {
        return PyLong_AsLong(&object);
    }

    template <>
    std::wstring_view PythonSystem::Cast<std::wstring_view>(PyObject& object)
    {
        Py_ssize_t size;
        wchar_t* str = PyUnicode_AsWideCharString(&object, &size);
        return std::wstring_view(str, size);
    }

    template <>
    std::span<const std::byte> PythonSystem::Cast<std::span<const std::byte>>(PyObject& object)
    {
        const Py_ssize_t size = PyBytes_Size(&object);
        const char* data = PyBytes_AsString(&object);
        return std::span<const std::byte>(reinterpret_cast<const std::byte*>(data), size);
    }
} // namespace AIHoloImager
