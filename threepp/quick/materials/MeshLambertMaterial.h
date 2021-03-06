//
// Created by byter on 12/14/17.
//

#ifndef THREEPPQ_MeshLambertMaterial_H
#define THREEPPQ_MeshLambertMaterial_H

#include <QColor>
#include <threepp/material/MeshLambertMaterial.h>
#include <threepp/quick/textures/Texture.h>
#include "Material.h"

namespace three {
namespace quick {

class MeshLambertMaterial : public Material
{
Q_OBJECT
  Q_PROPERTY(QColor color READ color WRITE setColor NOTIFY colorChanged)
  Q_PROPERTY(QColor emissive READ emissive WRITE setEmissive NOTIFY emissiveChanged)
  Q_PROPERTY(Texture *envMap READ envMap WRITE setEnvMap NOTIFY envMapChanged)

  QColor _color, _emissive;
  Texture *_envMap = nullptr;

  three::MeshLambertMaterial::Ptr _material;

protected:
  three::Material::Ptr material() const override {return _material;}

public:
  MeshLambertMaterial(three::MeshLambertMaterial::Ptr mat, QObject *parent=nullptr)
     : Material(material::Typer(this), parent), _material(mat) {}

  MeshLambertMaterial(QObject *parent=nullptr)
     : Material(material::Typer(this), parent) {}

  QColor color() const {return _color;}

  void setColor(const QColor &color) {
    if(_color != color) {
      _color = color;
      if(_material) {
        _material->color.set(color.redF(), color.greenF(), color.blueF());
        _material->needsUpdate = true;
      }
      emit colorChanged();
    }
  }

  QColor emissive() const {return _emissive;}

  void setEmissive(const QColor &emissive) {
    if(_emissive != emissive) {
      _emissive = emissive;
      if(_material) {
        _material->emissive.set(emissive.redF(), emissive.greenF(), emissive.blueF());
        _material->needsUpdate = true;
      }
      emit emissiveChanged();
    }
  }

  Texture *envMap() const {return _envMap;}

  void setEnvMap(Texture *envMap) {
    if(_envMap != envMap) {
      _envMap = envMap;
      if(_material) {
        _material->envMap = _envMap->getTexture();
        _material->needsUpdate = true;
      }
      emit envMapChanged();
    }
  }

  three::MeshLambertMaterial::Ptr createMaterial()
  {
    _material = three::MeshLambertMaterial::make();
    if(_color.isValid())
      _material->color = Color(_color.redF(), _color.greenF(), _color.blueF());

    setBaseProperties(_material);

    if(_envMap) {
      _material->envMap = _envMap->getTexture();
      if(!_material->envMap)
        qWarning() << "envMap is ignored";
    }

    return _material;
  }

  three::Material::Ptr getMaterial() override
  {
    return _material ? _material : createMaterial();
  }

signals:
  void colorChanged();
  void emissiveChanged();
  void envMapChanged();
};

}
}

#endif //THREEPPQ_MeshLambertMaterial_H
