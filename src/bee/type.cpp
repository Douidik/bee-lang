#include "type.hpp"
#include "ast.hpp"
#include "entity.hpp"
#include "expr.hpp"
#include "var.hpp"

namespace bee
{

f64 Atom_Type::max() const
{
    if (desc & Atom_Float)
    {
        switch (size)
        {
        case 2:
            return (f64)4'293'918'720.0;
        case 4:
            return (f64)3.402'823'466e+38;
        case 8:
            return (f64)1.797'693'134'862'315'7e+308;
        default:
            return (f64)0;
        }
    }
    else
    {
        return pow(2, size) - 1;
    }
}

} // namespace bee
