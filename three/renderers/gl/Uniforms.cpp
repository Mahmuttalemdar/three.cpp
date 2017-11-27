//
// Created by byter on 12.10.17.
//

#include "Uniforms.h"
#include "Renderer_impl.h"
#include <regex>

namespace three {
namespace gl {

using namespace std;

std::vector<int32_t> &Uniforms::allocTexUnits(Renderer_impl &renderer, size_t n)
{
  if(arrayCacheI32.find(n) == arrayCacheI32.end()) {
    arrayCacheI32.emplace(n, std::vector<int32_t>(n));
    arrayCacheI32[n].resize(n);
  }
  auto &r = arrayCacheI32[ n ];

  for ( size_t i = 0; i < n; ++i)
    r[i] = renderer.allocTextureUnit();

  return r;
}

#define MATCH_NAME(nm) if(name == #nm) return UniformName::nm;

UniformName toUniformName(string name)
{
  MATCH_NAME(cube)
  MATCH_NAME(equirect)
  MATCH_NAME(flip)
  MATCH_NAME(opacity)
  MATCH_NAME(diffuse)
  MATCH_NAME(emissive)
  MATCH_NAME(specular)
  MATCH_NAME(shininess)
  MATCH_NAME(projectionMatrix)
  MATCH_NAME(viewMatrix)
  MATCH_NAME(modelViewMatrix)
  MATCH_NAME(normalMatrix)
  MATCH_NAME(modelMatrix)
  MATCH_NAME(logDepthBufFC)
  MATCH_NAME(boneMatrices)
  MATCH_NAME(bindMatrix)
  MATCH_NAME(bindMatrixInverse)
  MATCH_NAME(toneMappingExposure)
  MATCH_NAME(toneMappingWhitePoint)
  MATCH_NAME(cameraPosition)
  MATCH_NAME(map)
  MATCH_NAME(uvTransform)
  MATCH_NAME(alphaMap)
  MATCH_NAME(specularMap)
  MATCH_NAME(envMap)
  MATCH_NAME(flipEnvMap)
  MATCH_NAME(reflectivity)
  MATCH_NAME(refractionRatio)
  MATCH_NAME(aoMap)
  MATCH_NAME(aoMapIntensity)
  MATCH_NAME(lightMap)
  MATCH_NAME(lightMapIntensity)
  MATCH_NAME(emissiveMap)
  MATCH_NAME(bumpMap)
  MATCH_NAME(bumpScale)
  MATCH_NAME(normalMap)
  MATCH_NAME(normalScale)
  MATCH_NAME(displacementMap)
  MATCH_NAME(displacementScale)
  MATCH_NAME(displacementBias)
  MATCH_NAME(roughnessMap)
  MATCH_NAME(metalnessMap)
  MATCH_NAME(gradientMap)
  MATCH_NAME(roughness)
  MATCH_NAME(metalness)
  MATCH_NAME(clearCoat)
  MATCH_NAME(clearCoatRoughness)
  MATCH_NAME(envMapIntensity)
  MATCH_NAME(fogDensity)
  MATCH_NAME(fogNear)
  MATCH_NAME(fogFar)
  MATCH_NAME(fogColor)
  MATCH_NAME(ambientLightColor)
  MATCH_NAME(direction)
  MATCH_NAME(color)
  MATCH_NAME(shadow)
  MATCH_NAME(shadowBias)
  MATCH_NAME(shadowRadius)
  MATCH_NAME(shadowMapSize)
  MATCH_NAME(size)
  MATCH_NAME(scale)
  MATCH_NAME(dashSize)
  MATCH_NAME(totalSize)
  MATCH_NAME(referencePosition)
  MATCH_NAME(nearDistance)
  MATCH_NAME(farDistance)
  MATCH_NAME(clippingPlanes)
  MATCH_NAME(directionalLights)
  MATCH_NAME(spotLights)
  MATCH_NAME(rectAreaLights)
  MATCH_NAME(pointLights)
  MATCH_NAME(hemisphereLights)
  MATCH_NAME(directionalShadowMap)
  MATCH_NAME(directionalShadowMatrix)
  MATCH_NAME(spotShadowMap)
  MATCH_NAME(spotShadowMatrix)
  MATCH_NAME(pointShadowMap)
  MATCH_NAME(pointShadowMatrix)
  MATCH_NAME(distance)
  MATCH_NAME(position)
  MATCH_NAME(coneCos)
  MATCH_NAME(penumbraCos)
  MATCH_NAME(decay)

  throw std::invalid_argument(std::string("unknown variable ")+name);
}

void Uniforms::parseUniform(GLuint program, unsigned index, UniformContainer *container)
{
  static regex rex(R"(([\w\d_]+)(\])?(\[|\.)?)");

  GLchar uname[100];
  GLsizei length;
  GLint size;
  GLenum type;

  _fn->glGetActiveUniform( program, index, 100, &length, &size, &type, uname);
  GLint addr = _fn->glGetUniformLocation(program, uname);

  string name(uname);
  sregex_iterator rex_it(name.cbegin(), name.cend(), rex);
  sregex_iterator rex_end;

  while(rex_it != rex_end) {
    smatch match = *rex_it;

    UniformName id = toUniformName(match[1]);
    bool isIndex = match[2] == "]";
    string subscript = match[3];

    if(!match[3].matched || match[3].second == name.end()) {
      // bare name or "pure" bottom-level array "[0]" suffix
      container->add(match[3].matched ?
                     ArrayUniform::make(_fn, id, (UniformType)type, addr) :
                     Uniform::make(_fn, id, (UniformType)type, addr));
    }
    else {
      // step into inner node / create it in case it doesn't exist
      if(container->_map.find(id) == container->_map.end()) {
        StructuredUniform::Ptr next = StructuredUniform::make(_fn, id, (UniformType)type, addr);
        container->add(next);
      }
      container = container->_map[id]->asContainer();
      assert(container != nullptr);
    }
    rex_it++;
  }
}

void Uniform::setValue(const Texture::Ptr &texture, Renderer_impl &renderer )
{
  unsigned unit = renderer.allocTextureUnit();
  _fn->glUniform1i( _addr, unit );
  renderer.setTexture2D(texture, unit );
}

void Uniform::setValue(const ImageCubeTexture::Ptr &texture, Renderer_impl &renderer )
{
  unsigned unit = renderer.allocTextureUnit();
  _fn->glUniform1i( _addr, unit );
  renderer.setTextureCube(texture, unit );
}

void Uniform::setValue(const DataCubeTexture::Ptr &texture, Renderer_impl &renderer )
{
  unsigned unit = renderer.allocTextureUnit();
  _fn->glUniform1i( _addr, unit );
  renderer.setTextureCube(texture, unit );
}

}
}
