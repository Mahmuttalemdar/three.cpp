//
// Created by byter on 05.08.17.
//

#ifndef THREEPP_RENDERERIMPL
#define THREEPP_RENDERERIMPL

#include <cstdio>
#include <threepp/renderers/OpenGLRenderer.h>
#include <threepp/light/Light.h>
#include <threepp/math/Frustum.h>
#include <threepp/objects/Sprite.h>
#include <threepp/objects/LensFlare.h>
#include <threepp/camera/ArrayCamera.h>
#include "RenderTarget.h"
#include "BufferRenderer.h"
#include "SpriteRenderer.h"
#include "FlareRenderer.h"
#include "Helpers.h"
#include "State.h"
#include "Extensions.h"
#include "Capabilities.h"
#include "Properties.h"
#include "Lights.h"
#include "Clipping.h"
#include "RenderLists.h"
#include "ShadowMap.h"
#include "MorphTargets.h"
#include "Programs.h"
#include "Background.h"

#include <QOpenGLShaderProgram>

namespace three {

class ImmediateRenderObject;

namespace gl {

class DeferredCalls;

class Renderer_impl : public OpenGLRenderer, public QOpenGLExtraFunctions
{
  friend class Programs;
  friend class Program;
  friend class RenderTargetExternal;
  friend class DeferredCalls;

  DeferredCalls *_deferredCalls;

protected:
  std::vector<Light::Ptr> _lightsArray;
  std::vector<Light::Ptr> _shadowsArray;

  //var currentRenderList = null;

  std::vector<Sprite::Ptr> _spritesArray;
  std::vector<LensFlare::Ptr> _flaresArray;

  // scene graph
  bool _sortObjects = true;

  // user-defined clipping
  std::vector<math::Plane> _clippingPlanes;
  bool _localClippingEnabled = false;

  // physically based shading
  float gammaFactor = 2.0f;	// for backwards compatibility
  bool gammaInput = false;
  bool gammaOutput = false;

  // physical lights
  bool _physicallyCorrectLights = false;

  // tone mapping
  ToneMapping _toneMapping = ToneMapping::Linear;
  float _toneMappingExposure = 1.0;
  float _toneMappingWhitePoint = 1.0;

  // morphs
  unsigned _maxMorphTargets = 8;
  unsigned _maxMorphNormals = 4;

  // internal state cache
  Renderer::Target::Ptr _currentRenderTarget = nullptr;
  GLuint _currentFramebuffer = UINT_MAX;
  int _currentMaterialId = -1;

  static const std::tuple<size_t, GLuint, bool> no_program;
  std::tuple<size_t, GLuint, bool> _currentGeometryProgram = no_program;

  Camera::Ptr _currentCamera;
  ArrayCamera::Ptr _currentArrayCamera;

  math::Vector4 _currentViewport;
  math::Vector4 _currentScissor;
  bool _currentScissorTest = false;

  Capabilities::Parameters _parameters;
  Extensions _extensions;
  Capabilities _capabilities;
  Properties _properties;

  Programs _programs;

  Textures _textures;

  DefaultBufferRenderer _bufferRenderer;

  IndexedBufferRenderer _indexedBufferRenderer;

  SpriteRenderer _spriteRenderer;

  FlareRenderer _flareRenderer;

  Background _background;

  bool _premultipliedAlpha = true;

  MemoryInfo _infoMemory;
  RenderInfo _infoRender;

  ShadowMap _shadowMap;

  Attributes _attributes;

  Geometries _geometries;

  Lights _lights;

  Objects _objects;

  MorphTargets _morphTargets;

  unsigned _usedTextureUnits = 0;

  float _width;
  float _height;

  float _pixelRatio = 1;

  math::Vector4 _viewport;
  math::Vector4 _scissor;
  bool _scissorTest = false;

  // frustum
  math::Frustum _frustum;

  // clipping
  Clipping _clipping;
  bool _clippingEnabled = false;

  // camera matrices cache
  math::Matrix4 _projScreenMatrix;
  math::Vector3 _vector3;

  RenderLists _renderLists;
  RenderList *_currentRenderList = nullptr;

  float getTargetPixelRatio()
  {
    return _currentRenderTarget ? _pixelRatio : 1;
  }

  gl::State _state;

  void initContext() override;

  void initMaterial(Material::Ptr material, Fog::Ptr fog, Object3D::Ptr object);

  void projectObject(Object3D::Ptr object, Camera::Ptr camera, bool sortObjects );

  void doRender(const Scene::Ptr &scene,
                const Camera::Ptr &camera,
                const Renderer::Target::Ptr &renderTarget,
                bool forceClear) override;

  void renderObjects(RenderList::iterator renderIterator,
                     Scene::Ptr scene,
                     Camera::Ptr camera,
                     Material::Ptr overrideMaterial);

  void renderObject(Object3D::Ptr object, Scene::Ptr scene, Camera::Ptr camera, BufferGeometry::Ptr geometry,
                    Material::Ptr material, const Group *group );

  Program::Ptr setProgram(Camera::Ptr camera, Fog::Ptr fog, Material::Ptr material, Object3D::Ptr object );

  void releaseMaterialProgramReference(Material &material);

  void renderObjectImmediate(ImmediateRenderObject &object, Program::Ptr program, Material::Ptr material);

  void renderBufferImmediate(ImmediateRenderObject &object, Program::Ptr program, Material::Ptr material);

  void setupVertexAttributes(Material::Ptr material, Program::Ptr program, BufferGeometry::Ptr geometry, unsigned startIndex=0);

public:
  using Ptr = std::shared_ptr<Renderer_impl>;

  Renderer_impl(size_t width, size_t height, float pixelRatio, bool premultipliedAlpha=true);
  ~Renderer_impl();

  gl::State &state() {return _state;}

  Renderer_impl &setRenderTarget(const Renderer::Target::Ptr renderTarget);

  const Renderer::Target::Ptr getRenderTarget() {return _currentRenderTarget;}

  bool localClippingEnabled() const {return _localClippingEnabled;}

  void clear(bool color, bool depth, bool stencil);

  unsigned allocTextureUnit();

  std::vector<GLint> allocTextureUnits(size_t count);

  void renderBufferDirect(Camera::Ptr camera,
                          Fog::Ptr fog,
                          BufferGeometry::Ptr geometry,
                          Material::Ptr material,
                          Object3D::Ptr object,
                          const Group *group);

  void setTexture2D(Texture::Ptr texture, GLuint slot);
  void setTextureCube(Texture::Ptr texture, GLuint slot);

  ShadowMap &shadowMap() {return _shadowMap;}

  const Renderer::Target::Ptr &currentRenderTarget() {
    return _currentRenderTarget;
  }

  void setShadowMapType(three::ShadowMapType type) override {
    _shadowMap.setEnabled(type != three::ShadowMapType::NoShadow);
    _shadowMap.setType(type);
  }

  void setFaceCulling(CullFace cullFace ) override
  {
    _state.setCullFace(cullFace);
    if(cullFace == CullFace::None)
      _state.setFaceDirection(FrontFaceDirection::Undefined);
  }

  void setFaceDirection( FrontFaceDirection frontFaceDirection ) override
  {
    _state.setFaceDirection(frontFaceDirection);
    if(frontFaceDirection == FrontFaceDirection::Undefined)
      _state.setCullFace(CullFace::None);
  }

  Renderer_impl &setClearColor(const Color &color, float alpha) override {
    _background.setClearColor(color, alpha);
    return *this;
  }

  void clear() override;

  Renderer_impl &setSize(size_t width, size_t height, bool setViewport) override;

  Renderer_impl &setViewport(size_t x, size_t y, size_t width, size_t height) override;
};

}
}
#endif //THREEPP_RENDERERIMPL
