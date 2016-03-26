/*
 * ExtendsNode.cpp
 *
 *      Author: jc
 */
#include "ExtendsNode.h"

#include <algorithm>
#include <functional>
#include <regex>

#include <Common.h>
#include <Exception/TemplateSyntaxError.h>
#include <IO/FileReader.h>
#include <Parser/Fragment.h>
#include <Parser/Parser.h>

#include "BlockNode.h"
#include "Root.h"

using namespace std::placeholders;

namespace RedZone {

ExtendsNode::ExtendsNode() :
   Node( true ) {
}

void ExtendsNode::render( Writer * stream, Context * context ) const {
   renderChildren( stream, context, m_nodesToRender );
}

void ExtendsNode::processFragment( Fragment const * fragment ) {
   static std::regex const splitter( R"(^extends\s+(.+)$)" );
   std::smatch match;
   std::string cleaned = fragment->clean();
   if( ! std::regex_match( cleaned, match, splitter ) ) {
      throw TemplateSyntaxError( fragment->clean() );
   }
   std::string path = match[ 1 ];
   std::vector< std::string > const & allParserPaths = Parser::paths();
   auto found = std::find_if( allParserPaths.begin(), allParserPaths.end(),
      std::bind( &isReadableFile, std::bind(
         std::function< std::string( std::string const &, std::string const & ) >( strConcat ), _1, path ) ) );
   if( found == allParserPaths.end() ) {
      throw Exception( "Extending failed. Cannot open template file " + path );
   }
   path = *found + path;
   FileReader reader( path );  // FIXME: get rid of specific Reader creating
   static Parser parser;
   m_parentRoot.reset( parser.loadFromStream( &reader ) );
}

void ExtendsNode::exitScope( std::string const & endTag ) {
   if( endTag != "endextends" )
      throw TemplateSyntaxError( endTag );

   auto extendsNodes = m_parentRoot->childrenByName< ExtendsNode >( name() );
   if( extendsNodes.size() > 1 )
      throw Exception( "Parent template has more than one 'extends' blocks" );
   if( extendsNodes.size() && m_parentRoot->children()[ 0 ]->name() != name() )
      throw Exception( "Extends node must be the first in " + m_parentRoot->id() );
   std::vector< std::shared_ptr< Node > > parentChildNodes;
   if( extendsNodes.size() ) {
      parentChildNodes = weakToShared( extendsNodes[ 0 ] )->m_nodesToRender;
   }
   else {
      parentChildNodes = m_parentRoot->children();
   }
   auto blocks = childrenByName< BlockNode >( "Block" );
   for( std::shared_ptr< Node > node: parentChildNodes ) {
      if( node->name() == "Block" ) {
         std::shared_ptr< BlockNode > blockNode = std::dynamic_pointer_cast< BlockNode >( node );
         std::string blockName = blockNode->blockName();
         auto found = std::find_if( blocks.begin(), blocks.end(),
            [ & ]( auto block ) -> bool { return blockName == weakToShared( block )->blockName(); } );
         if( found != blocks.end() ) {
            m_nodesToRender.push_back( 
               std::dynamic_pointer_cast< Node >( weakToShared( * found ) ) );
            continue;
         }
         m_nodesToRender.push_back( node );
      }
      else {
         m_nodesToRender.push_back( node );
      }
   }
}

std::string ExtendsNode::name() const {
   return "Extends";
}

ExtendsNode::~ExtendsNode() {
}

} /* namespace RedZone */
