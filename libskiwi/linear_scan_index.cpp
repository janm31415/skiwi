#include "linear_scan_index.h"
#include "visitor.h"

SKIWI_BEGIN

namespace
  {
  struct linear_scan_index_visitor : public base_visitor<linear_scan_index_visitor>
    {
    uint64_t pre_index, index;

    virtual bool _previsit(Fixnum& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(Flonum& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(Nil& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(String& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(Symbol& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(True& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(False& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(Character& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(Nop& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(Quote& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(Variable& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(Begin& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(FunCall& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(If& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(Lambda& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(Let& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(PrimitiveCall& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(ForeignCall& obj) { obj.pre_scan_index = pre_index++; return true; }
    virtual bool _previsit(Set& obj) { obj.pre_scan_index = pre_index++; return true; }

    virtual void _postvisit(Fixnum& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(Flonum& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(Nil& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(String& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(Symbol& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(True& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(False& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(Character& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(Nop& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(Quote& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(Variable& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(Begin& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(FunCall& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(If& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(Lambda& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(Let& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(PrimitiveCall& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(ForeignCall& obj) { obj.scan_index = pre_index; }
    virtual void _postvisit(Set& obj) { obj.scan_index = pre_index; }
    };
  }

void compute_linear_scan_index(Program& prog)
  {
  linear_scan_index_visitor lsiv;
  lsiv.index = 0;
  lsiv.pre_index = 0;
  visitor<Program, linear_scan_index_visitor>::visit(prog, &lsiv);
  prog.linear_scan_indices_computed = true;
  }

SKIWI_END
