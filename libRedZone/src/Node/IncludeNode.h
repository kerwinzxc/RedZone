/*
 * IncludeNode.h
 *
 *      Author: jc
 */

#pragma once

#include <string>
#include <vector>

#include "Node.h"

namespace RedZone {

class Root;

class RZ_API IncludeNode : public Node {
public:
   IncludeNode();

   virtual void render( Writer * stream, Context * context ) const;

   virtual void processFragment( Fragment const * fragment );

   virtual std::string name() const;

   virtual ~IncludeNode();

protected:
   std::string m_includeExpr;
};

} /* namespace RedZone */
