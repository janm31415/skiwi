#pragma once

#include "namespace.h"

#include <map>
#include <memory>
#include <stdint.h>
#include <algorithm>
#include <string>

SKIWI_BEGIN
  
template <class TEntry>
class environment
  {
  public:
    environment(const std::shared_ptr<environment<TEntry>>& outer) : p_outer(outer) {}

    bool has(const std::string& name)
      {
      auto it = env.find(name);
      if (it != env.end())
        return true;
      auto p_outer_copy = p_outer;
      while (p_outer_copy)
        {
        it = p_outer_copy->env.find(name);
        if (it != p_outer_copy->env.end())
          return true;
        p_outer_copy = p_outer_copy->p_outer;
        }
      return false;
      }

    bool find(TEntry& e, const std::string& name)
      {
      auto it = env.find(name);
      if (it != env.end())
        {
        e = it->second;
        return true;
        }
      auto p_outer_copy = p_outer;
      while (p_outer_copy)
        {
        it = p_outer_copy->env.find(name);
        if (it != p_outer_copy->env.end())
          {
          e = it->second;
          return true;
          }
        p_outer_copy = p_outer_copy->p_outer;
        }
      return false;
      }

    template <class Pred>
    bool find_if(std::pair<std::string, TEntry>& out, Pred p)
      {
      auto it = std::find_if(env.begin(), env.end(), p);
      if (it != env.end())
        {
        out = *it;
        return true;
        }
      //if (p_outer)
      //  {
      //  return p_outer->find_if(out, p);
      //  }
      auto p_outer_copy = p_outer;
      while (p_outer_copy)
        {
        it = std::find_if(p_outer_copy->env.begin(), p_outer_copy->env.end(), p);
        if (it != p_outer_copy->env.end())
          {
          out = *it;
          return true;
          }
        p_outer_copy = p_outer_copy->p_outer;
        }
      return false;
      }

    bool replace(const std::string& name, TEntry e)
      {
      auto it = env.find(name);
      if (it != env.end())
        {
        it->second = e;
        return true;
        }
      auto p_outer_copy = p_outer;
      while (p_outer_copy)
        {
        it = p_outer_copy->env.find(name);
        if (it != p_outer_copy->env.end())
          {
          it->second = e;
          return true;
          }
        p_outer_copy = p_outer_copy->p_outer;
        }
      //if (p_outer)
      //  return p_outer->replace(name, e);
      return false;
      }

    void remove(const std::string& name)
      {
      auto it = env.find(name);
      if (it != env.end())
        {
        env.erase(it);
        }
      //if (p_outer)
      //  p_outer->remove(name);
      auto p_outer_copy = p_outer;
      while (p_outer_copy)
        {
        it = p_outer_copy->env.find(name);
        if (it != p_outer_copy->env.end())
          {
          p_outer_copy->env.erase(it);
          }
        p_outer_copy = p_outer_copy->p_outer;
        }
      }

    void push(const std::string& name, TEntry e)
      {
      env[name] = e;
      }

    void push_outer(const std::string& name, TEntry e)
      {
      /*
      if (p_outer)
        p_outer->push_outer(name, e);
      else
        env[name] = e;
      */
      if (p_outer)
        {
        auto p_outer_copy = p_outer;
        while (p_outer_copy->p_outer)
          p_outer_copy = p_outer_copy->p_outer;
        p_outer_copy->env[name] = e;
        }
      else
        env[name] = e;
      }

    typename std::map<std::string, TEntry>::iterator begin()
      {
      return env.begin();
      }

    typename std::map<std::string, TEntry>::iterator end()
      {
      return env.end();
      }

    void rollup()
      {
      /*
      if (p_outer)
        {
        p_outer->rollup();
        auto it = p_outer->begin();
        auto it_end = p_outer->end();
        for (; it != it_end; ++it)
          env.insert(*it);
        p_outer.reset();
        }
      */
      while (p_outer)
        {
        auto it = p_outer->begin();
        auto it_end = p_outer->end();
        for (; it != it_end; ++it)
          env.insert(*it);
        p_outer = p_outer->p_outer;
        }
      }

  private:
    template <class T>
    friend std::shared_ptr<environment<T>> make_deep_copy(const std::shared_ptr<environment<T>>& env);
    std::map<std::string, TEntry> env;
    std::shared_ptr<environment<TEntry>> p_outer;
  };

template <class TEntry>
std::shared_ptr<environment<TEntry>> make_deep_copy(const std::shared_ptr<environment<TEntry>>& env)
  {
  if (!env.get())
    return env;
  std::shared_ptr<environment<TEntry>> out = std::make_shared<environment<TEntry>>(*env);
  //if (out->p_outer)
  //  out->p_outer = make_deep_copy(out->p_outer);
  auto* p_outer_copy = &(out->p_outer);
  while (*p_outer_copy)
    { // this loop is not tested anywhere yet
    *p_outer_copy = std::make_shared<environment<TEntry>>(*p_outer_copy);
    p_outer_copy = &((*p_outer_copy)->p_outer);
    }
  return out;
  }

SKIWI_END
