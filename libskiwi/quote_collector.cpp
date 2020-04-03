#include "quote_collector.h"

#include <cassert>
#include <sstream>

#include "visitor.h"
#include "reader.h"

SKIWI_BEGIN

namespace
  {
  struct quote_collector_visitor : public base_visitor<quote_collector_visitor>
    {
    std::map<std::string, cell> collected_quotes;

    virtual bool _previsit(Quote& q)
      {
      std::stringstream str;
      str << q.arg;
      collected_quotes[str.str()] = q.arg;
      return true;
      }
    };
  }

void collect_quotes(Program& prog, repl_data& data)
  {
  assert(!prog.quotes_collected);
  quote_collector_visitor qcv;
  visitor<Program, quote_collector_visitor>::visit(prog, &qcv);  
  prog.quotes_collected = true;
  prog.quotes.clear();
  for (auto q : qcv.collected_quotes)
    {    
    if (data.quote_to_index.find(q.first) == data.quote_to_index.end())
      prog.quotes.insert(q);
    }  
  }

SKIWI_END
