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

#include "owl/owl.h"
#include <stdio.h>
#include <stdlib.h>

struct TrianglesVars {
  OWL_int3   *index;
  OWL_float3 *vertex;
  OWL_float3  color;
};

struct SphereVars {
  OWL_float3  center;
  float       radius;
  OWL_float3  color;
};

struct RenderFrameVars {
  OWLDeviceTraversable world;
  OWLDeviceBuffer2D    frameBuffer;
  OWL_float3 camera_org;
  OWL_float3 camera_dir_00;
  OWL_float3 camera_dir_dx;
  OWL_float3 camera_dir_dy;
  OWL_float3 bgColor;
};




/* ------------------------------------------------------------------ */
/* the scene we are to render */
/* ------------------------------------------------------------------ */
/* size of image */
OWL_int2   image_fbSize  = { 800,600 };
OWL_float3 image_bgColor = { .2f, .2f, .2f };
  
/* bottom plane - a gray quad */
OWL_float3 quad_vertex[4] = {
                             { -10.f, 0.f, -10.f }, 
                             { -10.f, 0.f, +10.f }, 
                             { +10.f, 0.f, +10.f }, 
                             { +10.f, 0.f, -10.f }, 
};
OWL_int3   quad_index[2] = {
                            { 0,1,2 },
                            { 0,2,3 }
};
OWL_float3 quad_color = { .8f, .8f, .8f };

/* center sphere, green */
OWL_float3 sphere_center = { 0.f, 1.f, 0.f };
OWL_float  sphere_radius = 1.f;
OWL_float3 sphere_color  = { 0.f, 1.f, 0.f };
  
/* camera, centered on sphere */
OWL_float3 camera_at   = { 0.f, 1.f, 0.f };
OWL_float3 camera_up   = { 0.f, 1.f, 0.f };
OWL_float3 camera_from = {-4.f, 3.f,-2.f };



extern char ptxCode[1];

int main(int ac, char **av)
{
  OWLContext  context  = owlContextCreate();
  OWLBuffer   frameBuffer = owlContextCreateBuffer(context,OWL_FLOAT3,3,NULL);

  OWLModule   module = owlModuleCreate(context,ptxCode);
  
  // ==================================================================
  // create base quad as triangles geometry
  // ==================================================================

  // ------------------------------------------------------------------
  // define type
  // ------------------------------------------------------------------

  OWLVarDecl diffuseTrianglesVars[] =
    {
     { "vertex", OWL_BUFFER_POINTER, OWL_OFFSETOF(TrianglesVars,vertex)},
     { "color",  OWL_FLOAT3,         OWL_OFFSETOF(TrianglesVars,color)},
    };
  OWLGeomType diffuseTrianglesType
    = owlGeomTypeCreate(context,
                        OWL_GEOMETRY_TRIANGLES,
                        sizeof(struct TrianglesVars),
                        diffuseTrianglesVars,
                        sizeof(diffuseTrianglesVars)/sizeof(OWLVarDecl));
  /* owlGeomTypeDeclareVariable(diffuseTrianglesType,"vertex",OWL_BUFFER_POINTER, */
  /*                                OWL_OFFSETOF(TrianglesVars,vertex)); */
  /* owlGeomTypeDeclareVariable(diffuseTrianglesType,"index", OWL_BUFFER_POINTER, */
  /*                                OWL_OFFSETOF(TrianglesVars,index)); */
  /* owlGeomTypeDeclareVariable(diffuseTrianglesType,"color",OWL_FLOAT3, */
  /*                                OWL_OFFSETOF(TrianglesVars,color)); */

  owlGeomTypeSetClosestHit(diffuseTrianglesType,
                           OWL_ALL_RAY_TYPES,
                           module,"__closesthit_triangles");
  // ------------------------------------------------------------------
  // now _create_ and _set_ the geometry
  // ------------------------------------------------------------------
  
  // create actual quads geometry
  OWLGeom quad = owlContextCreateGeom(context,diffuseTrianglesType);
  
  // create and set vertex buffer
  OWLBuffer   vertices
    = owlContextCreateBuffer(context,OWL_FLOAT3,4,quad_vertex);
  owlTrianglesSetVertices(quad,vertices);
  owlBufferRelease(vertices);

  // create and set index buffer
  OWLBuffer   indices
    = owlContextCreateBuffer(context,OWL_INT3,2,quad_index);
  owlTrianglesSetIndices(quad,indices);
  owlBufferRelease(indices);
  
  // create and set color
  OWLVariable quadColor = owlGeomGetVariable(quad,"color");
  owlVariableSet3fv(quadColor,&quad_color.x);
  owlVariableRelease(quadColor);

  // ------------------------------------------------------------------
  // ... and put into a trace-able group
  // ------------------------------------------------------------------
  OWLGroup quadGroup = owlContextCreateGeomGroup(context,1,&quad);
  owlGeomRelease(quad);

  // ==================================================================
  // create and set sphere
  // ==================================================================

  // ------------------------------------------------------------------
  // define type
  // ------------------------------------------------------------------
  OWLVarDecl diffuseSphereVars[] =
    {
     { "center", OWL_FLOAT3, OWL_OFFSETOF(SphereVars,center)},
     { "color",  OWL_FLOAT3, OWL_OFFSETOF(SphereVars,color)},
     { "radius", OWL_FLOAT,  OWL_OFFSETOF(SphereVars,radius)},
    };
  OWLGeomType diffuseSphereType
    = owlGeomTypeCreate(context,
                        OWL_GEOMETRY_USER,
                        sizeof(struct SphereVars),
                        diffuseSphereVars,
                        sizeof(diffuseSphereVars)/sizeof(OWLVarDecl));
  
  owlGeomTypeSetClosestHit(diffuseSphereType,
                           OWL_ALL_RAY_TYPES,
                           module,"__closesthit_sphere");
  
  // ------------------------------------------------------------------
  // now _create_ and _set_ the geometry
  // ------------------------------------------------------------------
  
  // create actual sphere geometry
  OWLGeom sphere
    = owlContextCreateGeom(context,diffuseSphereType);
  
  // create and set color
  OWLVariable sphereColor
    = owlGeomGetVariable(sphere,"color");
  owlVariableSet3fv(sphereColor,&sphere_color.x);
  owlVariableRelease(sphereColor);

  // create and set center
  OWLVariable sphereCenter
    = owlGeomGetVariable(sphere,"center");
  owlVariableSet3fv(sphereCenter,&sphere_center.x);
  owlVariableRelease(sphereCenter);
  
  // create and set radius
  OWLVariable sphereRadius
    = owlGeomGetVariable(sphere,"radius");
  owlVariableSet1f(sphereRadius,sphere_radius);
  owlVariableRelease(sphereRadius);

  // ------------------------------------------------------------------
  // ... and put into a trace-able group
  // ------------------------------------------------------------------
  OWLGroup sphereGroup
    = owlContextCreateGeomGroup(context,1,&sphere);
  owlGeomRelease(sphere);
  
  // ==================================================================
  // create toplevel groupt hat contains both ...
  // ==================================================================
  OWLGroup worldGroup
    = owlContextCreateInstanceGroup(context,2);
  owlInstanceGroupSetChild(worldGroup,0,sphereGroup);
  owlInstanceGroupSetChild(worldGroup,1,quadGroup);
  owlGroupRelease(quadGroup);
  owlGroupRelease(sphereGroup);

  // ==================================================================
  // create and configure launch proram
  // ==================================================================
  OWLVarDecl renderFrameVars[] =
    {
     { "bgColor",     OWL_FLOAT3, OWL_OFFSETOF(RenderFrameVars,bgColor)},
     { "world",       OWL_GROUP,  OWL_OFFSETOF(RenderFrameVars,world)},
     { "frameBuffer", OWL_BUFFER, OWL_OFFSETOF(RenderFrameVars,frameBuffer)},
    };
  OWLRayGen renderFrame
    = owlContextCreateRayGen(context,
                                 /* code to run: */
                                 module,"renderFrame",
                                 /* size of variables struct */
                                 sizeof(struct RenderFrameVars),
                                 renderFrameVars,
                                 sizeof(renderFrameVars)/sizeof(OWLVarDecl));
  
  
  OWLVariable bgColorVar
    = owlRayGenGetVariable(renderFrame,"bgColor");
  owlVariableSet3fv(bgColorVar,&image_bgColor.x);
  owlVariableRelease(bgColorVar);

  OWLVariable worldVar
    = owlRayGenGetVariable(renderFrame,"world");
  owlVariableSetGroup(worldVar,worldGroup);
  owlVariableRelease(worldVar);

  OWLVariable fbVar
    = owlRayGenGetVariable(renderFrame,"frameBuffer");
  owlVariableSetBuffer(fbVar,frameBuffer);
  owlVariableRelease(fbVar);
  
  // ==================================================================
  // launch renderframe program
  // ==================================================================
  owlContextLaunch2D(context,renderFrame,image_fbSize.x,image_fbSize.y);

  owlBufferRelease(frameBuffer);
  owlRayGenRelease(renderFrame);
  owlGroupRelease(worldGroup);

  owlContextDestroy(context);
}
