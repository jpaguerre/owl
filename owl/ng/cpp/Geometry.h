// ======================================================================== //
// Copyright 2019 Ingo Wald                                                 //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#pragma once

#include "RegisteredObject.h"
#include "SBTObject.h"
#include "Module.h"
#include "Buffer.h"

namespace owl {

  struct Geom;
  
  struct GeomType : public SBTObjectType {
    typedef std::shared_ptr<GeomType> SP;
    
    GeomType(Context *const context,
                 size_t varStructSize,
                 const std::vector<OWLVarDecl> &varDecls);

    virtual std::string toString() const { return "GeomType"; }
    virtual void setClosestHitProgram(int rayType,
                                      Module::SP module,
                                      const std::string &progName)
    { IGNORING_THIS(); }

    virtual std::shared_ptr<Geom> createGeom() = 0;
  };

  struct TrianglesGeomType : public GeomType {
    typedef std::shared_ptr<TrianglesGeomType> SP;
    
    TrianglesGeomType(Context *const context,
                          size_t varStructSize,
                          const std::vector<OWLVarDecl> &varDecls)
      : GeomType(context,varStructSize,varDecls)
    {}

    virtual std::string toString() const { return "TriangleGeomType"; }
    virtual std::shared_ptr<Geom> createGeom() override;
  };

  struct UserGeomType : public GeomType {
    typedef std::shared_ptr<UserGeomType> SP;
    
    UserGeomType(Context *const context,
                     size_t varStructSize,
                     const std::vector<OWLVarDecl> &varDecls)
      : GeomType(context,varStructSize,varDecls)
    {}

    virtual std::string toString() const { return "UserGeomType"; }
    virtual std::shared_ptr<Geom> createGeom() override;
  };

  struct Geom : public SBTObject<GeomType> {
    typedef std::shared_ptr<Geom> SP;

    Geom(Context *const context,
             GeomType::SP geometryType);
    virtual std::string toString() const { return "Geom"; }
    
    GeomType::SP geometryType;
  };

  struct TrianglesGeom : public Geom {
    typedef std::shared_ptr<TrianglesGeom> SP;

    TrianglesGeom(Context *const context,
                      GeomType::SP geometryType)
      : Geom(context,geometryType)
    {}

    void setVertices(Buffer::SP vertices)
    { IGNORING_THIS(); }
    void setIndices(Buffer::SP indices)
    { IGNORING_THIS(); }
    virtual std::string toString() const { return "TrianglesGeom"; }
  };

  struct UserGeom : public Geom {
    typedef std::shared_ptr<UserGeom> SP;

    UserGeom(Context *const context,
                 GeomType::SP geometryType)
      : Geom(context,geometryType)
    {}
    virtual std::string toString() const { return "UserGeom"; }
  };
  
} // ::owl