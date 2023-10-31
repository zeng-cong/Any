#ifndef _ANY_H
#define _ANY_H
#include "Utility/UtilMacro.h"
#include <exception>
#include <iostream>
#include <type_traits>
#include <typeinfo>
#include <utility>
class CastException : public std::exception
{
  private:
    std::string _text;

  public:
    CastException(std::string text = "") : _text(text){};
    virtual ~CastException(){};
    virtual const char *what() const noexcept override
    {
        return _text.c_str();
    };
};
class Any final
{
  private:
    enum class DONTCREATEFLAG
    {
        FLAG
    };
    class Holder
    {
      public:
        void *_actualData;
        bool _isRef;
        bool _isClass;
        bool _isConst;

      public:
        Holder() : _actualData(nullptr), _isRef(false), _isClass(false), _isConst(false){};
        virtual ~Holder(){};
        virtual Holder *Clone() const = 0;
        virtual void Destory() = 0;
        virtual const std::type_info &ActualType() const = 0;
        virtual void operator=(const Holder &h) = 0;
        bool IsClass() const
        {
            return _isClass;
        }
        bool IsRef() const
        {
            return _isRef;
        }
        bool IsConst() const
        {
            return _isConst;
        }
        void SetData(void *data)
        {
            _actualData = data;
            _isRef = true;
        };
        template <typename T> T &Value()
        {
            return *static_cast<T *>(_actualData);
        };
        template <typename T> const T &Value() const
        {
            return *static_cast<T *>(_actualData);
        }

        template <typename T> void SetValue(T &&val)
        {
            Value<rmref(T)>() = std::forward<T>(val);
        }
        bool IsValid() const
        {
            return _actualData != nullptr;
        }
    };

    template <typename T> class Iholder final : public Holder
    {
      private:
        using data_type = rmref(T);

      public:
        Iholder(DONTCREATEFLAG flag)
        {
            _actualData = nullptr;
            _isRef = false;
            _isClass = std::is_class<T>::value;
            _isConst = std::is_const<T>::value;
        }
        Iholder() : Iholder(DONTCREATEFLAG::FLAG)
        {
            _actualData = new data_type;
        };

        Iholder(T &&val) : Iholder(DONTCREATEFLAG::FLAG)
        {
            _actualData = new typename std::remove_const<data_type>::type(std::move(val));
        }
        Iholder(const T &val) : Iholder(DONTCREATEFLAG::FLAG)
        {
            _actualData = new typename std::remove_const<data_type>::type(val);
        }
        virtual ~Iholder()
        {
            Destory();
        };
        virtual Holder *Clone() const override
        {
            if (!IsValid())
            {
                return nullptr;
            }
            return new Iholder<data_type>(Value<data_type>());
        };
        virtual void Destory() override
        {
            if (!IsRef())
            {
                auto d = static_cast<data_type *>(_actualData);
                SAFE_DELETE(d);
            }
            _actualData = nullptr;
        };
        virtual const std::type_info &ActualType() const override
        {
            return typeid(data_type);
        }
        virtual void operator=(const Holder &h) override
        {
            using data_type_uc = typename std::remove_const<data_type>::type;
            SetValue(*static_cast<data_type_uc *>(h._actualData));
        }
    };

  private:
    Holder *_data;

  public:
    template <typename T> Any(T &&val)
    {
        using data_type = rmref(T);
        _data = new Iholder<data_type>(std::forward<T>(val));
    }
    template <typename T, int size> Any(T (&val)[size]) : Any((T *)val)
    {
    }
    Any() : _data(nullptr){};
    Any(Any &&any)
    {
        _data = any._data;
        any._data = nullptr;
    };
    Any(const Any &&any)
    {
        _data = nullptr;
        if (any.IsValid())
        {
            _data = any._data->Clone();
        }
    }
    Any(Any &any)
    {
        _data = nullptr;
        if (any.IsValid())
        {
            _data = any._data->Clone();
        }
    }
    Any(const Any &any)
    {
        _data = nullptr;
        if (any.IsValid())
        {
            _data = any._data->Clone();
        }
    };

    virtual ~Any()
    {
        SAFE_DELETE(_data);
    };
    bool IsRef() const
    {
        return _data && _data->IsRef();
    }
    bool IsValid() const
    {
        return _data && _data->IsValid();
    }
    bool IsClass() const
    {
        return _data && _data->IsClass();
    }
    const std::type_info &ActualType() const
    {
        if (IsValid())
        {
            return _data->ActualType();
        }
        return typeid(void);
    }
    template <typename T, int size> void SetValue(T (&val)[size])
    {
        T *t = val;
        SetValue(t);
    }
    template <typename T> void SetValue(T &&val)
    {
        using data_type = rmref(T);
        using data_type_uc = typename std::remove_const<data_type>::type;
        if (ActualType() == typeid(data_type))
        {
            _data->Value<data_type_uc>() = std::forward<T>(val);
        }
        else
        {
            SAFE_DELETE(_data);
            _data = new Iholder<data_type>(std::forward<T>(val));
        }
    }
    template <typename T> T &Convert()
    {
        return _data->Value<T>();
    }
    template <typename T> T &Value()
    {
        using data_type = rmref(T);
        if (typeid(data_type) != ActualType())
        {
            throw CastException("error can't cast");
        }
        return _data->Value<data_type>();
    }
    template <typename T> const T &Value() const
    {
        using data_type = rmref(T);

        if (typeid(data_type) != ActualType())
        {
            throw CastException("error can't cast");
        }
        return _data->Value<data_type>();
    }
    Any &operator=(const Any &any)
    {
        if (ActualType() == any.ActualType())
        {
            *_data = *any._data;
        }
        else if (any.IsValid())
        {
            SAFE_DELETE(_data);
            _data = any._data->Clone();
        }
        else
        {
            SAFE_DELETE(_data);
        }
        return *this;
    }
    Any &operator=(Any &&any)
    {
        SAFE_DELETE(_data);
        _data = any._data;
        any._data = nullptr;
        return *this;
    }
    template <typename T> Any &operator=(T &&val)
    {
        SetValue(std::forward<T>(val));
        return *this;
    }

    template <typename T> operator T &()
    {
        return Value<T>();
    }
#ifdef __GNUC__
    template <typename T> operator T() const
    {
        return Value<T>();
    }
#endif
    template <typename T> operator const T &() const
    {
        return Value<T>();
    }
    template <typename T> void SetRefVal(T &val)
    {
        using data_type = rmref(T);
        SAFE_DELETE(_data);
        _data = new Iholder<data_type>(DONTCREATEFLAG::FLAG);
        _data->SetData(&val);
    }
    template <typename T> void SetType()
    {
        SAFE_DELETE(_data);
        using data_type = rmref(T);
        _data = new Iholder<data_type>();
    }
    template <typename T> bool EqualType()
    {
        return typeid(T) == ActualType();
    }
    template <typename T> bool operator==(const T &val) = delete;
    bool operator==(const Any &any) = delete;
    bool operator<(const Any &any) = delete;
    bool operator>(const Any &any) = delete;
    template <typename T> bool operator<(const T &val) = delete;
    template <typename T> bool operator>(const T &val) = delete;
};

#endif //_ANY_H