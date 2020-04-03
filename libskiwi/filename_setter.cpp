#include "filename_setter.h"

#include <cassert>
#include <sstream>
#include <fstream>

#include "visitor.h"

SKIWI_BEGIN

namespace
  {
  struct filename_setter_visitor : public base_visitor<filename_setter_visitor>
    {
    std::string filename;
    
    virtual void _postvisit(Fixnum& ob) { ob.filename = filename; }
    virtual void _postvisit(Flonum& ob) { ob.filename = filename; }
    virtual void _postvisit(Nil& ob) { ob.filename = filename; }
    virtual void _postvisit(String& ob) { ob.filename = filename; }
    virtual void _postvisit(Symbol& ob) { ob.filename = filename; }
    virtual void _postvisit(True& ob) { ob.filename = filename; }
    virtual void _postvisit(False& ob) { ob.filename = filename; }
    virtual void _postvisit(Character& ob) { ob.filename = filename; }
    virtual void _postvisit(Nop& ob) { ob.filename = filename; }
    virtual void _postvisit(Variable& ob) { ob.filename = filename; }    
    virtual void _postvisit(Begin& ob) { ob.filename = filename; }
    virtual void _postvisit(FunCall& ob) { ob.filename = filename; }
    virtual void _postvisit(If& ob) { ob.filename = filename; }
    virtual void _postvisit(Case& ob) { ob.filename = filename; }
    virtual void _postvisit(Cond& ob) { ob.filename = filename; }
    virtual void _postvisit(Do& ob) { ob.filename = filename; }
    virtual void _postvisit(Lambda& ob) { ob.filename = filename; }
    virtual void _postvisit(Let& ob) { ob.filename = filename; }    
    virtual void _postvisit(PrimitiveCall& ob) { ob.filename = filename; }
    virtual void _postvisit(ForeignCall& ob) { ob.filename = filename; }
    virtual void _postvisit(Set& ob) { ob.filename = filename; }
    virtual void _postvisit(Quote& ob) { ob.filename = filename; }
    };
  }

void set_filename(Program& prog, const std::string& filename)
  {
  filename_setter_visitor fsv;
  fsv.filename = filename;
  visitor<Program, filename_setter_visitor>::visit(prog, &fsv);
  }

SKIWI_END
