#pragma once

#include "compiler_options.h"

#include <functional>
#include <thread>
#include <cassert>
#include <exception>
#include <cstring>
#include <atomic>
#ifdef _WIN32
#include <windows.h>
#undef min
#undef max
#elif defined(unix)
#include <pthread.h>
#elif defined(__APPLE__)
#include <pthread.h>
#endif 


SKIWI_BEGIN

inline uint64_t get_thread_id()
  {
#ifdef _WIN32
  return (uint64_t)GetCurrentThreadId();
#elif defined(unix)
  return (uint64_t)pthread_self();
#elif defined(__APPLE__)
  uint64_t tid;
  pthread_threadid_np(NULL, &tid);
  return tid;
#endif 
  }


/*
  Taken from ppl.h, adapted to our custom parallel_for loop
*/
  template<typename _Ty>
class combinable
  {
  private:

    // Disable warning C4324: structure was padded due to __declspec(align())
    // This padding is expected and necessary.
#pragma warning(push)
#pragma warning(disable: 4324)
#ifdef _WIN32
    __declspec(align(64))
#endif
      struct _Node
      {
      uint64_t _M_key;
      _Ty _M_value;
      _Node* _M_chain;

      _Node(uint64_t _Key, _Ty _InitialValue)
        : _M_key(_Key),
        _M_value(std::move(_InitialValue)),
        _M_chain(nullptr)
        {
        }
      }
#ifndef _WIN32 // linux alignment in gcc
    __attribute__((aligned(64)))
#endif
      ;
#pragma warning(pop)

    static _Ty _DefaultInit()
      {
      return _Ty();
      }

  public:
    /// <summary>
    ///     Constructs a new <c>combinable</c> object.
    /// </summary>
    /// <remarks>
    ///     <para>The first constructor initializes new elements with the default constructor for the type <paramref name="_Ty"/>.</para>
    ///     <para>The second constructor initializes new elements using the initialization functor supplied as the
    ///           <paramref name="_FnInitialize"/> parameter.</para>
    ///     <para>The third constructor is the copy constructor.</para>
    /// </remarks>
    /// <seealso cref="Parallel Containers and Objects"/>
    /**/
    combinable()
      : _M_buckets(),
      _M_size(),
      _M_fnInitialize(_DefaultInit)
      {
      _InitNew();
      }

    /// <summary>
    ///     Constructs a new <c>combinable</c> object.
    /// </summary>
    /// <typeparam name="_Function">
    ///     The type of the initialization functor object.
    /// </typeparam>
    /// <param name="_FnInitialize">
    ///     A function which will be called to initialize each new thread-private value of the type <paramref name="_Ty"/>.
    ///     It must support a function call operator with the signature <c>_Ty ()</c>.
    /// </param>
    /// <remarks>
    ///     <para>The first constructor initializes new elements with the default constructor for the type <paramref name="_Ty"/>.</para>
    ///     <para>The second constructor initializes new elements using the initialization functor supplied as the
    ///           <paramref name="_FnInitialize"/> parameter.</para>
    ///     <para>The third constructor is the copy constructor.</para>
    /// </remarks>
    /// <seealso cref="Parallel Containers and Objects"/>
    /**/
    template <typename _Function>
    explicit combinable(_Function _FnInitialize)
      : _M_buckets(),
      _M_size(),
      _M_fnInitialize(std::move(_FnInitialize))
      {
      _InitNew();
      }

    /// <summary>
    ///     Constructs a new <c>combinable</c> object.
    /// </summary>
    /// <param name="_Other">
    ///     An existing <c>combinable</c> object to be copied into this one.
    /// </param>
    /// <remarks>
    ///     <para>The first constructor initializes new elements with the default constructor for the type <paramref name="_Ty"/>.</para>
    ///     <para>The second constructor initializes new elements using the initialization functor supplied as the
    ///           <paramref name="_FnInitialize"/> parameter.</para>
    ///     <para>The third constructor is the copy constructor.</para>
    /// </remarks>
    /// <seealso cref="Parallel Containers and Objects"/>
    /**/
    combinable(const combinable& _Other)
      : _M_buckets(),
      _M_size(_Other._M_size),
      _M_fnInitialize(_Other._M_fnInitialize) // throws
      {
      _M_buckets = _InitCopy(_Other);
      }

    /// <summary>
    ///     Assigns to a <c>combinable</c> object from another <c>combinable</c> object.
    /// </summary>
    /// <param name="_Other">
    ///     An existing <c>combinable</c> object to be copied into this one.
    /// </param>
    /// <returns>
    ///     A reference to this <c>combinable</c> object.
    /// </returns>
    /**/
    combinable& operator=(const combinable& _Other)
      {
      auto _Fn_initialize_copy = _Other._M_fnInitialize; // throws
      auto _New_buckets = _InitCopy(_Other); // throws
      // remaining ops cannot throw
      clear();
      delete[] _M_buckets;
      _M_buckets = _New_buckets;
      _M_fnInitialize.swap(_Fn_initialize_copy);
      _M_size = _Other._M_size;

      return *this;
      }

    /// <summary>
    ///     Destroys a <c>combinable</c> object.
    /// </summary>
    /**/
    ~combinable()
      {
      clear();
      delete[] _M_buckets;
      }

    /// <summary>
    ///     Returns a reference to the thread-private sub-computation.
    /// </summary>
    /// <returns>
    ///     A reference to the thread-private sub-computation.
    /// </returns>
    /// <seealso cref="Parallel Containers and Objects"/>
    /**/
    _Ty& local()
      {
      uint64_t _Key = (uint64_t)get_thread_id();
      size_t _Index;
      _Node* _ExistingNode = _FindLocalItem(_Key, &_Index);
      if (_ExistingNode == nullptr)
        {
        _ExistingNode = _AddLocalItem(_Key, _Index);
        }

      assert(_ExistingNode != nullptr);
      return _ExistingNode->_M_value;
      }

    /// <summary>
    ///     Returns a reference to the thread-private sub-computation.
    /// </summary>
    /// <param name="_Exists">
    ///     A reference to a boolean. The boolean value referenced by this argument will be
    ///     set to <c>true</c> if the sub-computation already existed on this thread, and set to
    ///     <c>false</c> if this was the first sub-computation on this thread.
    /// </param>
    /// <returns>
    ///     A reference to the thread-private sub-computation.
    /// </returns>
    /// <seealso cref="Parallel Containers and Objects"/>
    /**/
    _Ty& local(bool& _Exists)
      {
      uint64_t _Key = get_thread_id();
      size_t _Index;
      _Node* _ExistingNode = _FindLocalItem(_Key, &_Index);
      if (_ExistingNode == nullptr)
        {
        _Exists = false;
        _ExistingNode = _AddLocalItem(_Key, _Index);
        }
      else
        {
        _Exists = true;
        }

      assert(_ExistingNode != nullptr);
      return _ExistingNode->_M_value;
      }

    /// <summary>
    ///     Clears any intermediate computational results from a previous usage.
    /// </summary>
    /**/
    void clear()
      {
      for (size_t _Index = 0; _Index < _M_size; ++_Index)
        {
        _Node* _CurrentNode = _M_buckets[_Index];
        while (_CurrentNode != nullptr)
          {
          _Node* _NextNode = _CurrentNode->_M_chain;
          delete _CurrentNode;
          _CurrentNode = _NextNode;
          }
        }
      memset((void*)_M_buckets, 0, _M_size * sizeof _M_buckets[0]);
      }

    /// <summary>
    ///     Computes a final value from the set of thread-local sub-computations by calling the supplied combine functor.
    /// </summary>
    /// <typeparam name="_Function">
    ///     The type of the function object that will be invoked to combine two thread-local sub-computations.
    /// </typeparam>
    /// <param name="_FnCombine">
    ///     The functor that is used to combine the sub-computations. Its signature is <c>T (T, T)</c> or
    ///     <c>T (const T&amp;, const T&amp;)</c>, and it must be associative and commutative.
    /// </param>
    /// <returns>
    ///     The final result of combining all the thread-private sub-computations.
    /// </returns>
    /// <seealso cref="Parallel Containers and Objects"/>
    /**/
    template<typename _Function>
    _Ty combine(_Function _FnCombine) const
      {
      _Node* _CurrentNode = nullptr;
      size_t _Index;

      // Look for the first value in the set, and use (a copy of) that as the result.
      // This eliminates a single call (of unknown cost) to _M_fnInitialize.
      for (_Index = 0; _Index < _M_size; ++_Index)
        {
        _CurrentNode = _M_buckets[_Index];
        if (_CurrentNode != nullptr)
          {
          break;
          }
        }

      // No values... return the initializer value.
      if (_CurrentNode == nullptr)
        {
        return _M_fnInitialize();
        }

      // Accumulate the rest of the items in the current bucket.
      _Ty _Result = _CurrentNode->_M_value;
      for (_CurrentNode = _CurrentNode->_M_chain; _CurrentNode != nullptr; _CurrentNode = _CurrentNode->_M_chain)
        {
        _Result = _FnCombine(_Result, _CurrentNode->_M_value);
        }

      // Accumulate values from the rest of the buckets.
      assert(_Index < _M_size);
      for (++_Index; _Index < _M_size; ++_Index)
        {
        for (_CurrentNode = _M_buckets[_Index]; _CurrentNode != nullptr; _CurrentNode = _CurrentNode->_M_chain)
          {
          _Result = _FnCombine(_Result, _CurrentNode->_M_value);
          }
        }

      return _Result;
      }

    /// <summary>
    ///     Computes a final value from the set of thread-local sub-computations by calling the supplied combine functor
    ///     once per thread-local sub-computation. The final result is accumulated by the function object.
    /// </summary>
    /// <typeparam name="_Function">
    ///     The type of the function object that will be invoked to combine a single thread-local sub-computation.
    /// </typeparam>
    /// <param name="_FnCombine">
    ///     The functor that is used to combine one sub-computation. Its signature is <c>void (T)</c> or
    ///     <c>void (const T&amp;)</c>, and must be associative and commutative.
    /// </param>
    /// <seealso cref="Parallel Containers and Objects"/>
    /**/
    template<typename _Function>
    void combine_each(_Function _FnCombine) const
      {
      for (size_t _Index = 0; _Index < _M_size; ++_Index)
        {
        for (_Node* _CurrentNode = _M_buckets[_Index]; _CurrentNode != nullptr; _CurrentNode = _CurrentNode->_M_chain)
          {
          _FnCombine(_CurrentNode->_M_value);
          }
        }
      }

  private:
    void _InitNew()
      {
      _M_size = std::thread::hardware_concurrency();
      _M_buckets = new std::atomic<_Node*>[_M_size] {};
      }

    struct _InitCopyOp
      {
      std::unique_ptr<_Node* []> _M_new_buckets;
      size_t _M_index; // invariant: !_M_new_buckets || _M_index < _Size

      explicit _InitCopyOp(size_t _Size)
        : _M_new_buckets(),
        _M_index(0)
        {
        if (_Size != 0)
          {
          _M_new_buckets = std::make_unique<_Node* []>(_Size);
          }
        }

      _Node** _DoCopy(size_t _Size, const combinable& _Other)
        {
        for (; _M_index < _Size; ++_M_index)
          {
          for (_Node* _CurrentNode = _Other._M_buckets[_M_index]; _CurrentNode != nullptr;
            _CurrentNode = _CurrentNode->_M_chain)
            {
            // allocate node and push_front
            _Node* _NewNode = new _Node(_CurrentNode->_M_key, _CurrentNode->_M_value);
            _NewNode->_M_chain = _M_new_buckets[_M_index];
            _M_new_buckets[_M_index] = _NewNode;
            }
          }

        return _M_new_buckets.release(); // also muzzles destructor
        }

      ~_InitCopyOp()
        {
        if (_M_new_buckets)
          {
          // if we get here, an exception was thrown in _DoCopy; note we must back out including the
          // _M_index-th entry (where the exception was thrown), hence <=
          for (size_t _Next = 0; _Next <= _M_index; ++_Next)
            {
            _Node* _CurrentNode = _M_new_buckets[_Next];
            while (_CurrentNode)
              {
              const auto _NextNode = _CurrentNode->_M_chain;
              delete _CurrentNode;
              _CurrentNode = _NextNode;
              }
            }
          }
        }
      };

    static _Node** _InitCopy(const combinable& _Other)
      {
      _InitCopyOp _Op{ _Other._M_size };
      return _Op._DoCopy(_Other._M_size, _Other);
      }

    _Node* _FindLocalItem(uint64_t _Key, size_t* _PIndex)
      {
      assert(_PIndex != nullptr);

      *_PIndex = _Key % _M_size;

      // Search at this index for an existing value.
      for (_Node* _CurrentNode = _M_buckets[*_PIndex]; _CurrentNode != nullptr; _CurrentNode = _CurrentNode->_M_chain)
        {
        if (_CurrentNode->_M_key == _Key)
          {
          return _CurrentNode;
          }
        }

      return nullptr;
      }

    _Node* _AddLocalItem(uint64_t _Key, size_t _Index)
      {
      _Node* _NewNode = new _Node(_Key, _M_fnInitialize());
      _Node* _TopNode;
      do
        {
        _TopNode = _M_buckets[_Index];
        _NewNode->_M_chain = _TopNode;
        } //while (_InterlockedCompareExchangePointer(reinterpret_cast<void * volatile *>(&_M_buckets[_Index]), _NewNode, _TopNode) != _TopNode);
      while (!_M_buckets[_Index].compare_exchange_strong(_TopNode, _NewNode));
        return _NewNode;
      }

  private:
    std::atomic<_Node*> volatile* _M_buckets;
    size_t _M_size;
    std::function<_Ty()> _M_fnInitialize;
  };


template <class _Type, class TFunctor>
void parallel_for(_Type first, _Type last, TFunctor fun, const compiler_options& ops)
  {
  if (ops.parallel)
    {
    const _Type n_threads = (_Type)std::thread::hardware_concurrency();

    const _Type n = last - first;

    const _Type n_max_tasks_per_thread = (n / n_threads) + (n % n_threads == 0 ? 0 : 1);
    const _Type n_lacking_tasks = n_max_tasks_per_thread * n_threads - n;

    auto inner_loop = [&](const _Type thread_index)
      {
      const _Type n_lacking_tasks_so_far = n_threads > (thread_index + n_lacking_tasks) ? 0 : (thread_index + n_lacking_tasks) - n_threads;
      const _Type inclusive_start_index = thread_index * n_max_tasks_per_thread - n_lacking_tasks_so_far;
      const _Type exclusive_end_index = inclusive_start_index + n_max_tasks_per_thread - (thread_index + n_lacking_tasks >= n_threads ? 1 : 0);

      for (_Type k = inclusive_start_index; k < exclusive_end_index; ++k)
        {
        fun(k + first);
        }
      };
    std::vector<std::thread> threads;
    for (_Type j = 0; j < n_threads; ++j) { threads.push_back(std::thread(inner_loop, j)); }
    for (auto& t : threads) { t.join(); }

    }
  else
    {
    for (_Type i = first; i != last; ++i)
      fun(i);
    }
  }

SKIWI_END
