/* -*-c++-*- OpenSceneGraph - Copyright (C) 1998-2006 Robert Osfield
 *
 * This library is open source and may be redistributed and/or modified under
 * the terms of the OpenSceneGraph Public License (OSGPL) version 0.0 or
 * (at your option) any later version.  The full license is in LICENSE file
 * included with this distribution, and on the openscenegraph.org website.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * OpenSceneGraph Public License for more details.
*/
#include <stdlib.h>

#include <osg/GeometryNew>
#include <osg/ArrayDispatchers>
#include <osg/Notify>

using namespace osg;

GeometryNew::GeometryNew()
{
    // temporary test
    // setSupportsDisplayList(false);
}

GeometryNew::GeometryNew(const GeometryNew& geometry,const CopyOp& copyop):
    Drawable(geometry,copyop),
    _vertexArray(copyop(geometry._vertexArray.get())),
    _normalArray(copyop(geometry._normalArray.get())),
    _colorArray(copyop(geometry._colorArray.get())),
    _secondaryColorArray(copyop(geometry._secondaryColorArray.get())),
    _fogCoordArray(copyop(geometry._fogCoordArray.get()))
{
    // temporary test
    // setSupportsDisplayList(false);

    for(PrimitiveSetList::const_iterator pitr=geometry._primitives.begin();
        pitr!=geometry._primitives.end();
        ++pitr)
    {
        PrimitiveSet* primitive = copyop(pitr->get());
        if (primitive) _primitives.push_back(primitive);
    }

    for(ArrayList::const_iterator titr=geometry._texCoordList.begin();
        titr!=geometry._texCoordList.end();
        ++titr)
    {
        _texCoordList.push_back(copyop(titr->get()));
    }

    for(ArrayList::const_iterator vitr=geometry._vertexAttribList.begin();
        vitr!=geometry._vertexAttribList.end();
        ++vitr)
    {
        _vertexAttribList.push_back(copyop(vitr->get()));
    }

    if ((copyop.getCopyFlags() & osg::CopyOp::DEEP_COPY_ARRAYS))
    {
        if (_useVertexBufferObjects)
        {
            // copying of arrays doesn't set up buffer objects so we'll need to force
            // Geometry to assign these, we'll do this by switching off VBO's then renabling them.
            setUseVertexBufferObjects(false);
            setUseVertexBufferObjects(true);
        }
    }

}

GeometryNew::~GeometryNew()
{
    // do dirty here to keep the getGLObjectSizeHint() estimate on the ball
    dirtyDisplayList();

    // no need to delete, all automatically handled by ref_ptr :-)
}

#define ARRAY_NOT_EMPTY(array) (array!=0 && array->getNumElements()!=0) 

bool GeometryNew::empty() const
{
    if (!_primitives.empty()) return false;
    if (ARRAY_NOT_EMPTY(_vertexArray.get())) return false;
    if (ARRAY_NOT_EMPTY(_normalArray.get())) return false;
    if (ARRAY_NOT_EMPTY(_colorArray.get())) return false;
    if (ARRAY_NOT_EMPTY(_secondaryColorArray.get())) return false;
    if (ARRAY_NOT_EMPTY(_fogCoordArray.get())) return false;
    if (!_texCoordList.empty()) return false;
    if (!_vertexAttribList.empty()) return false;
    return true;
}

void GeometryNew::setVertexArray(Array* array)
{
    _vertexArray = array;

    dirtyDisplayList();
    dirtyBound();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

void GeometryNew::setNormalArray(Array* array)
{
    _normalArray = array;

    dirtyDisplayList();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

void GeometryNew::setColorArray(Array* array)
{
    _colorArray = array;

    dirtyDisplayList();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

void GeometryNew::setSecondaryColorArray(Array* array)
{
    _secondaryColorArray = array;

    dirtyDisplayList();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

void GeometryNew::setFogCoordArray(Array* array)
{
    _fogCoordArray = array;

    dirtyDisplayList();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

#define SET_BINDING(array)\
    if (!array) \
    { \
        OSG_NOTICE<<"Warning, can't assign attribute binding as no has been array assigned to set binding for."<<std::endl; \
        return; \
    } \
    if (array->getBinding() == static_cast<osg::Array::Binding>(ab)) return; \
    array->setBinding(static_cast<osg::Array::Binding>(ab));
        

#define GET_BINDING(array) (array!=0 ? static_cast<AttributeBinding>(array->getBinding()) : BIND_OFF) 


void GeometryNew::setNormalBinding(AttributeBinding ab)
{
    SET_BINDING(_normalArray.get())

    dirtyDisplayList();
}

GeometryNew::AttributeBinding GeometryNew::getNormalBinding() const
{
    return GET_BINDING(_normalArray.get());
}
        
void GeometryNew::setColorBinding(AttributeBinding ab)
{
    SET_BINDING(_colorArray.get())

    dirtyDisplayList();
}

GeometryNew::AttributeBinding GeometryNew::getColorBinding() const
{
    return GET_BINDING(_colorArray.get());
}
        
void GeometryNew::setSecondaryColorBinding(AttributeBinding ab)
{
    SET_BINDING(_secondaryColorArray.get())

    dirtyDisplayList();
}

GeometryNew::AttributeBinding GeometryNew::getSecondaryColorBinding() const
{
    return GET_BINDING(_secondaryColorArray.get());
}
        
void GeometryNew::setFogCoordBinding(AttributeBinding ab)
{
    SET_BINDING(_fogCoordArray.get())

    dirtyDisplayList();
}

GeometryNew::AttributeBinding GeometryNew::getFogCoordBinding() const
{
    return GET_BINDING(_fogCoordArray.get());
}


void GeometryNew::setTexCoordArray(unsigned int index,Array* array)
{
    if (_texCoordList.size()<=index)
        _texCoordList.resize(index+1);

    _texCoordList[index] = array;

    // do we set to array BIND_PER_VERTEX?

    dirtyDisplayList();

    if (_useVertexBufferObjects && array)
    {
        addVertexBufferObjectIfRequired(array);
    }
}

Array* GeometryNew::getTexCoordArray(unsigned int index)
{
    if (index<_texCoordList.size()) return _texCoordList[index].get();
    else return 0;
}

const Array* GeometryNew::getTexCoordArray(unsigned int index) const
{
    if (index<_texCoordList.size()) return _texCoordList[index].get();
    else return 0;
}


void GeometryNew::setVertexAttribArray(unsigned int index, Array* array)
{
    if (_vertexAttribList.size()<=index)
        _vertexAttribList.resize(index+1);

    _vertexAttribList[index] = array;

    dirtyDisplayList();

    if (_useVertexBufferObjects && array) addVertexBufferObjectIfRequired(array);
}

Array *GeometryNew::getVertexAttribArray(unsigned int index)
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].get();
    else return 0;
}

const Array *GeometryNew::getVertexAttribArray(unsigned int index) const
{
    if (index<_vertexAttribList.size()) return _vertexAttribList[index].get();
    else return 0;
}

void GeometryNew::setVertexAttribBinding(unsigned int index,AttributeBinding ab)
{
    if (index<_vertexAttribList.size() && _vertexAttribList[index].valid())
    {
        if (_vertexAttribList[index]->getBinding()==static_cast<osg::Array::Binding>(ab)) return;

        _vertexAttribList[index]->setBinding(static_cast<osg::Array::Binding>(ab));
        
        dirtyDisplayList();
    }
    else
    {
        OSG_NOTICE<<"Warning, can't assign attribute binding as no has been array assigned to set binding for."<<std::endl;
    }
}

GeometryNew::AttributeBinding GeometryNew::getVertexAttribBinding(unsigned int index) const
{
    if (index<_vertexAttribList.size() && _vertexAttribList[index].valid()) return static_cast<AttributeBinding>(_vertexAttribList[index]->getBinding());
    else return BIND_OFF;
}

void GeometryNew::setVertexAttribNormalize(unsigned int index,GLboolean norm)
{
    if (index<_vertexAttribList.size() && _vertexAttribList[index].valid())
    {
        _vertexAttribList[index]->setNormalize(norm);
        
        dirtyDisplayList();
    }
}

GLboolean GeometryNew::getVertexAttribNormalize(unsigned int index) const
{
    if (index<_vertexAttribList.size() && _vertexAttribList[index].valid()) return _vertexAttribList[index]->getNormalize();
    else return GL_FALSE;
}

bool GeometryNew::addPrimitiveSet(PrimitiveSet* primitiveset)
{
    if (primitiveset)
    {
        if (_useVertexBufferObjects) addElementBufferObjectIfRequired(primitiveset);

        _primitives.push_back(primitiveset);
        dirtyDisplayList();
        dirtyBound();
        return true;
    }
    OSG_WARN<<"Warning: invalid index i or primitiveset passed to osg::GeometryNew::addPrimitiveSet(i,primitiveset), ignoring call."<<std::endl;
    return false;
}

bool GeometryNew::setPrimitiveSet(unsigned int i,PrimitiveSet* primitiveset)
{
    if (i<_primitives.size() && primitiveset)
    {
        if (_useVertexBufferObjects) addElementBufferObjectIfRequired(primitiveset);

        _primitives[i] = primitiveset;
        dirtyDisplayList();
        dirtyBound();
        return true;
    }
    OSG_WARN<<"Warning: invalid index i or primitiveset passed to osg::GeometryNew::setPrimitiveSet(i,primitiveset), ignoring call."<<std::endl;
    return false;
}

bool GeometryNew::insertPrimitiveSet(unsigned int i,PrimitiveSet* primitiveset)
{

    if (primitiveset)
    {
        if (_useVertexBufferObjects) addElementBufferObjectIfRequired(primitiveset);

        if (i<_primitives.size())
        {
            _primitives.insert(_primitives.begin()+i,primitiveset);
            dirtyDisplayList();
            dirtyBound();
            return true;
        }
        else if (i==_primitives.size())
        {
            return addPrimitiveSet(primitiveset);
        }

    }
    OSG_WARN<<"Warning: invalid index i or primitiveset passed to osg::GeometryNew::insertPrimitiveSet(i,primitiveset), ignoring call."<<std::endl;
    return false;
}

void GeometryNew::setPrimitiveSetList(const PrimitiveSetList& primitives)
{
    _primitives = primitives;
    if (_useVertexBufferObjects)
    {
        for (unsigned int primitiveSetIndex=0;primitiveSetIndex<_primitives.size();++primitiveSetIndex)
        {
            addElementBufferObjectIfRequired(_primitives[primitiveSetIndex].get());
        }

    }
    dirtyDisplayList(); dirtyBound();
}

bool GeometryNew::removePrimitiveSet(unsigned int i, unsigned int numElementsToRemove)
{
    if (numElementsToRemove==0) return false;

    if (i<_primitives.size())
    {
        if (i+numElementsToRemove<=_primitives.size())
        {
            _primitives.erase(_primitives.begin()+i,_primitives.begin()+i+numElementsToRemove);
        }
        else
        {
            // asking to delete too many elements, report a warning, and delete to
            // the end of the primitive list.
            OSG_WARN<<"Warning: osg::GeometryNew::removePrimitiveSet(i,numElementsToRemove) has been asked to remove more elements than are available,"<<std::endl;
            OSG_WARN<<"         removing on from i to the end of the list of primitive sets."<<std::endl;
            _primitives.erase(_primitives.begin()+i,_primitives.end());
        }

        dirtyDisplayList();
        dirtyBound();
        return true;
    }
    OSG_WARN<<"Warning: invalid index i passed to osg::GeometryNew::removePrimitiveSet(i,numElementsToRemove), ignoring call."<<std::endl;
    return false;
}

unsigned int GeometryNew::getPrimitiveSetIndex(const PrimitiveSet* primitiveset) const
{
    for (unsigned int primitiveSetIndex=0;primitiveSetIndex<_primitives.size();++primitiveSetIndex)
    {
        if (_primitives[primitiveSetIndex]==primitiveset) return primitiveSetIndex;
    }
    return _primitives.size(); // node not found.
}

unsigned int GeometryNew::getGLObjectSizeHint() const
{
    unsigned int totalSize = 0;
    if (_vertexArray.valid()) totalSize += _vertexArray->getTotalDataSize();

    if (_normalArray.valid()) totalSize += _normalArray->getTotalDataSize();

    if (_colorArray.valid()) totalSize += _colorArray->getTotalDataSize();

    if (_secondaryColorArray.valid()) totalSize += _secondaryColorArray->getTotalDataSize();

    if (_fogCoordArray.valid()) totalSize += _fogCoordArray->getTotalDataSize();


    unsigned int unit;
    for(unit=0;unit<_texCoordList.size();++unit)
    {
        const Array* array = _texCoordList[unit].get();
        if (array) totalSize += array->getTotalDataSize();

    }

    bool handleVertexAttributes = true;
    if (handleVertexAttributes)
    {
        unsigned int index;
        for( index = 0; index < _vertexAttribList.size(); ++index )
        {
            const Array* array = _vertexAttribList[index].get();
            if (array) totalSize += array->getTotalDataSize();
        }
    }

    for(PrimitiveSetList::const_iterator itr=_primitives.begin();
        itr!=_primitives.end();
        ++itr)
    {

        totalSize += 4*(*itr)->getNumIndices();

    }


    // do a very simply mapping of display list size proportional to vertex datasize.
    return totalSize;
}

bool GeometryNew::getArrayList(ArrayList& arrayList) const
{
    unsigned int startSize = arrayList.size();

    if (_vertexArray.valid()) arrayList.push_back(_vertexArray.get());
    if (_normalArray.valid()) arrayList.push_back(_normalArray.get());
    if (_colorArray.valid()) arrayList.push_back(_colorArray.get());
    if (_secondaryColorArray.valid()) arrayList.push_back(_secondaryColorArray.get());
    if (_fogCoordArray.valid()) arrayList.push_back(_fogCoordArray.get());

    for(unsigned int unit=0;unit<_texCoordList.size();++unit)
    {
        Array* array = _texCoordList[unit].get();
        if (array) arrayList.push_back(array);
    }

    for(unsigned int  index = 0; index < _vertexAttribList.size(); ++index )
    {
        Array* array = _vertexAttribList[index].get();
        if (array) arrayList.push_back(array);
    }

    return arrayList.size()!=startSize;
}

bool GeometryNew::getDrawElementsList(DrawElementsList& drawElementsList) const
{
    unsigned int startSize = drawElementsList.size();

    for(PrimitiveSetList::const_iterator itr = _primitives.begin();
        itr != _primitives.end();
        ++itr)
    {
        osg::DrawElements* de = (*itr)->getDrawElements();
        if (de) drawElementsList.push_back(de);
    }

    return drawElementsList.size()!=startSize;
}

void GeometryNew::addVertexBufferObjectIfRequired(osg::Array* array)
{
    if (_useVertexBufferObjects)
    {
        if (!array->getVertexBufferObject())
        {
            array->setVertexBufferObject(getOrCreateVertexBufferObject());
        }
    }
}

void GeometryNew::addElementBufferObjectIfRequired(osg::PrimitiveSet* primitiveSet)
{
    if (_useVertexBufferObjects)
    {
        osg::DrawElements* drawElements = primitiveSet->getDrawElements();
        if (drawElements && !drawElements->getElementBufferObject())
        {
            drawElements->setElementBufferObject(getOrCreateElementBufferObject());
        }
    }
}

osg::VertexBufferObject* GeometryNew::getOrCreateVertexBufferObject()
{
    ArrayList arrayList;
    getArrayList(arrayList);

    ArrayList::iterator vitr;
    for(vitr = arrayList.begin();
        vitr != arrayList.end();
        ++vitr)
    {
        osg::Array* array = vitr->get();
        if (array->getVertexBufferObject()) return array->getVertexBufferObject();
    }

    return new osg::VertexBufferObject;
}

osg::ElementBufferObject* GeometryNew::getOrCreateElementBufferObject()
{
    DrawElementsList drawElementsList;
    getDrawElementsList(drawElementsList);

    DrawElementsList::iterator deitr;
    for(deitr = drawElementsList.begin();
        deitr != drawElementsList.end();
        ++deitr)
    {
        osg::DrawElements* elements = *deitr;
        if (elements->getElementBufferObject()) return elements->getElementBufferObject();
    }

    return new osg::ElementBufferObject;
}

void GeometryNew::setUseVertexBufferObjects(bool flag)
{
    // flag = true;

    // OSG_NOTICE<<"GeometryNew::setUseVertexBufferObjects("<<flag<<")"<<std::endl;

    if (_useVertexBufferObjects==flag) return;

    Drawable::setUseVertexBufferObjects(flag);

    ArrayList arrayList;
    getArrayList(arrayList);

    DrawElementsList drawElementsList;
    getDrawElementsList(drawElementsList);

    typedef std::vector<osg::VertexBufferObject*>  VertexBufferObjectList;
    typedef std::vector<osg::ElementBufferObject*>  ElementBufferObjectList;

    if (_useVertexBufferObjects)
    {
        if (!arrayList.empty())
        {

            VertexBufferObjectList vboList;

            osg::ref_ptr<osg::VertexBufferObject> vbo;

            ArrayList::iterator vitr;
            for(vitr = arrayList.begin();
                vitr != arrayList.end() && !vbo;
                ++vitr)
            {
                osg::Array* array = vitr->get();
                if (array->getVertexBufferObject()) vbo = array->getVertexBufferObject();
            }

            if (!vbo) vbo = new osg::VertexBufferObject;

            for(vitr = arrayList.begin();
                vitr != arrayList.end();
                ++vitr)
            {
                osg::Array* array = vitr->get();
                if (!array->getVertexBufferObject()) array->setVertexBufferObject(vbo.get());
            }
        }

        if (!drawElementsList.empty())
        {
            ElementBufferObjectList eboList;

            osg::ref_ptr<osg::ElementBufferObject> ebo;

            DrawElementsList::iterator deitr;
            for(deitr = drawElementsList.begin();
                deitr != drawElementsList.end();
                ++deitr)
            {
                osg::DrawElements* elements = *deitr;
                if (elements->getElementBufferObject()) ebo = elements->getElementBufferObject();
            }

            if (!ebo) ebo = new osg::ElementBufferObject;

            for(deitr = drawElementsList.begin();
                deitr != drawElementsList.end();
                ++deitr)
            {
                osg::DrawElements* elements = *deitr;
                if (!elements->getElementBufferObject()) elements->setElementBufferObject(ebo.get());
            }
        }
    }
    else
    {
        for(ArrayList::iterator vitr = arrayList.begin();
            vitr != arrayList.end();
            ++vitr)
        {
            osg::Array* array = vitr->get();
            if (array->getVertexBufferObject()) array->setVertexBufferObject(0);
        }

        for(DrawElementsList::iterator deitr = drawElementsList.begin();
            deitr != drawElementsList.end();
            ++deitr)
        {
            osg::DrawElements* elements = *deitr;
            if (elements->getElementBufferObject()) elements->setElementBufferObject(0);
        }
    }
}

void GeometryNew::dirtyDisplayList()
{
    Drawable::dirtyDisplayList();
}

void GeometryNew::resizeGLObjectBuffers(unsigned int maxSize)
{
    Drawable::resizeGLObjectBuffers(maxSize);

    ArrayList arrays;
    if (getArrayList(arrays))
    {
        for(ArrayList::iterator itr = arrays.begin();
            itr != arrays.end();
            ++itr)
        {
            (*itr)->resizeGLObjectBuffers(maxSize);
        }
    }

    DrawElementsList drawElements;
    if (getDrawElementsList(drawElements))
    {
        for(DrawElementsList::iterator itr = drawElements.begin();
            itr != drawElements.end();
            ++itr)
        {
            (*itr)->resizeGLObjectBuffers(maxSize);
        }
    }
}

void GeometryNew::releaseGLObjects(State* state) const
{
    Drawable::releaseGLObjects(state);

    ArrayList arrays;
    if (getArrayList(arrays))
    {
        for(ArrayList::iterator itr = arrays.begin();
            itr != arrays.end();
            ++itr)
        {
            (*itr)->releaseGLObjects(state);
        }
    }

    DrawElementsList drawElements;
    if (getDrawElementsList(drawElements))
    {
        for(DrawElementsList::iterator itr = drawElements.begin();
            itr != drawElements.end();
            ++itr)
        {
            (*itr)->releaseGLObjects(state);
        }
    }

}

void GeometryNew::compileGLObjects(RenderInfo& renderInfo) const
{
    bool useVertexArrays = _supportsVertexBufferObjects &&
                           _useVertexBufferObjects &&
                           renderInfo.getState()->isVertexBufferObjectSupported();
    if (useVertexArrays)
    {
        // OSG_NOTICE<<"GeometryNew::compileGLObjects() use VBO's "<<this<<std::endl;
        State& state = *renderInfo.getState();
        unsigned int contextID = state.getContextID();
        GLBufferObject::Extensions* extensions = GLBufferObject::getExtensions(contextID, true);
        if (!extensions) return;

        typedef std::set<BufferObject*> BufferObjects;
        BufferObjects bufferObjects;

        // first collect all the active unique BufferObjects
        if (_vertexArray.valid() && _vertexArray->getBufferObject()) bufferObjects.insert(_vertexArray->getBufferObject());
        if (_normalArray.valid() && _normalArray->getBufferObject()) bufferObjects.insert(_normalArray->getBufferObject());
        if (_colorArray.valid() && _colorArray->getBufferObject()) bufferObjects.insert(_colorArray->getBufferObject());
        if (_secondaryColorArray.valid() && _secondaryColorArray->getBufferObject()) bufferObjects.insert(_secondaryColorArray->getBufferObject());
        if (_fogCoordArray.valid() && _fogCoordArray->getBufferObject()) bufferObjects.insert(_fogCoordArray->getBufferObject());

        for(ArrayList::const_iterator itr = _texCoordList.begin();
            itr != _texCoordList.end();
            ++itr)
        {
            if (itr->valid() && (*itr)->getBufferObject()) bufferObjects.insert((*itr)->getBufferObject());
        }

        for(ArrayList::const_iterator itr = _vertexAttribList.begin();
            itr != _vertexAttribList.end();
            ++itr)
        {
            if (itr->valid() && (*itr)->getBufferObject()) bufferObjects.insert((*itr)->getBufferObject());
        }

        for(PrimitiveSetList::const_iterator itr = _primitives.begin();
            itr != _primitives.end();
            ++itr)
        {
            if ((*itr)->getBufferObject()) bufferObjects.insert((*itr)->getBufferObject());
        }

        //osg::ElapsedTime timer;

        // now compile any buffer objects that require it.
        for(BufferObjects::iterator itr = bufferObjects.begin();
            itr != bufferObjects.end();
            ++itr)
        {
            GLBufferObject* glBufferObject = (*itr)->getOrCreateGLBufferObject(contextID);
            if (glBufferObject && glBufferObject->isDirty())
            {
                // OSG_NOTICE<<"Compile buffer "<<glBufferObject<<std::endl;
                glBufferObject->compileBuffer();
            }
        }

        // OSG_NOTICE<<"Time to compile "<<timer.elapsedTime_m()<<"ms"<<std::endl;

        // unbind the BufferObjects
        extensions->glBindBuffer(GL_ARRAY_BUFFER_ARB,0);
        extensions->glBindBuffer(GL_ELEMENT_ARRAY_BUFFER_ARB,0);

    }
    else
    {
        Drawable::compileGLObjects(renderInfo);
    }
}

void GeometryNew::drawImplementation(RenderInfo& renderInfo) const
{
    State& state = *renderInfo.getState();

    bool checkForGLErrors = state.getCheckForGLErrors()==osg::State::ONCE_PER_ATTRIBUTE;
    if (checkForGLErrors) state.checkGLErrors("start of GeometryNew::drawImplementation()");

    bool usingVertexBufferObjects = _useVertexBufferObjects && state.isVertexBufferObjectSupported();
    bool handleVertexAttributes = !_vertexAttribList.empty();

    ArrayDispatchers& arrayDispatchers = state.getArrayDispatchers();

    arrayDispatchers.reset();
    arrayDispatchers.setUseVertexAttribAlias(state.getUseVertexAttributeAliasing());
    arrayDispatchers.setUseGLBeginEndAdapter(false);

    arrayDispatchers.activateNormalArray(_normalArray.get());
    arrayDispatchers.activateColorArray(_colorArray.get());
    arrayDispatchers.activateSecondaryColorArray(_secondaryColorArray.get());
    arrayDispatchers.activateFogCoordArray(_fogCoordArray.get());

    if (handleVertexAttributes)
    {
        for(unsigned int unit=0;unit<_vertexAttribList.size();++unit)
        {
            arrayDispatchers.activateVertexAttribArray(unit, _vertexAttribList[unit].get());
        }
    }

    // dispatch any attributes that are bound overall
    arrayDispatchers.dispatch(BIND_OVERALL,0);

    state.lazyDisablingOfVertexAttributes();

    // set up arrays
    if( _vertexArray.valid() )
        state.setVertexPointer(_vertexArray.get());

    if (_normalArray.valid() && _normalArray->getBinding()==osg::Array::BIND_PER_VERTEX)
        state.setNormalPointer(_normalArray.get());

    if (_colorArray.valid() && _colorArray->getBinding()==osg::Array::BIND_PER_VERTEX)
        state.setColorPointer(_colorArray.get());

    if (_secondaryColorArray.valid() && _secondaryColorArray->getBinding()==osg::Array::BIND_PER_VERTEX)
        state.setSecondaryColorPointer(_secondaryColorArray.get());

    if (_fogCoordArray.valid() && _fogCoordArray->getBinding()==osg::Array::BIND_PER_VERTEX)
        state.setFogCoordPointer(_fogCoordArray.get());

    for(unsigned int unit=0;unit<_texCoordList.size();++unit)
    {
        const Array* array = _texCoordList[unit].get();
        if (array) state.setTexCoordPointer(unit,array);
    }

    if( handleVertexAttributes )
    {
        for(unsigned int index = 0; index < _vertexAttribList.size(); ++index )
        {
            const Array* array = _vertexAttribList[index].get();
            if(array && array->getBinding()==osg::Array::BIND_PER_VERTEX)
            {
                state.setVertexAttribPointer( index, array, _vertexAttribList[index]->getNormalize() );
            }
        }
    }

    state.applyDisablingOfVertexAttributes();

    bool bindPerPrimitiveSetActive = arrayDispatchers.active(BIND_PER_PRIMITIVE_SET);

    if (checkForGLErrors) state.checkGLErrors("GeometryNew::drawImplementation() after vertex arrays setup.");

    ///////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
    //
    // draw the primitives themselves.
    //
    for(unsigned int primitiveSetNum=0; primitiveSetNum!=_primitives.size(); ++primitiveSetNum)
    {
        // dispatch any attributes that are bound per primitive
        if (bindPerPrimitiveSetActive) arrayDispatchers.dispatch(BIND_PER_PRIMITIVE_SET, primitiveSetNum);

        const PrimitiveSet* primitiveset = _primitives[primitiveSetNum].get();

        primitiveset->draw(state, usingVertexBufferObjects);
    }

    // unbind the VBO's if any are used.
    state.unbindVertexBufferObject();
    state.unbindElementBufferObject();

    if (checkForGLErrors) state.checkGLErrors("end of GeometryNew::drawImplementation().");
}

class AttributeFunctorArrayVisitor : public ArrayVisitor
{
    public:

        AttributeFunctorArrayVisitor(Drawable::AttributeFunctor& af):
            _af(af),
            _type(0) {}

        virtual ~AttributeFunctorArrayVisitor() {}

        virtual void apply(ByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(ShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(IntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(UByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(UShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(UIntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec4ubArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(FloatArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec2Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec3Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec4Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(DoubleArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec2dArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec3dArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(Vec4dArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }


        inline void applyArray(Drawable::AttributeType type,Array* array)
        {
            if (array)
            {
                _type = type;
                array->accept(*this);
            }
        }

    protected:

        AttributeFunctorArrayVisitor& operator = (const AttributeFunctorArrayVisitor&) { return *this; }
        Drawable::AttributeFunctor&   _af;
        Drawable::AttributeType       _type;
};

void GeometryNew::accept(AttributeFunctor& af)
{
    AttributeFunctorArrayVisitor afav(af);

    if (_vertexArray.valid())
    {
        afav.applyArray(VERTICES,_vertexArray.get());
    }
    else if (_vertexAttribList.size()>0)
    {
        OSG_INFO<<"GeometryNew::accept(AttributeFunctor& af): Using vertex attribute instead"<<std::endl;
        afav.applyArray(VERTICES,_vertexAttribList[0].get());
    }

    afav.applyArray(NORMALS,_normalArray.get());
    afav.applyArray(COLORS,_colorArray.get());
    afav.applyArray(SECONDARY_COLORS,_secondaryColorArray.get());
    afav.applyArray(FOG_COORDS,_fogCoordArray.get());

    for(unsigned unit=0;unit<_texCoordList.size();++unit)
    {
        afav.applyArray((AttributeType)(TEXTURE_COORDS_0+unit),_texCoordList[unit].get());
    }

    for(unsigned int index=0; index<_vertexAttribList.size(); ++index)
    {
        afav.applyArray(index,_vertexAttribList[index].get());
    }
}

class ConstAttributeFunctorArrayVisitor : public ConstArrayVisitor
{
    public:

        ConstAttributeFunctorArrayVisitor(Drawable::ConstAttributeFunctor& af):
            _af(af),
            _type(0) {}

        virtual ~ConstAttributeFunctorArrayVisitor() {}

        virtual void apply(const ByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const ShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const IntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const UByteArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const UShortArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const UIntArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec4ubArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const FloatArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec2Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec3Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec4Array& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const DoubleArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec2dArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec3dArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }
        virtual void apply(const Vec4dArray& array) {  if (!array.empty()) _af.apply(_type,array.size(),&(array.front())); }


        inline void applyArray(Drawable::AttributeType type,const Array* array)
        {
            if (array)
            {
                _type = type;
                array->accept(*this);
            }
        }

protected:

        ConstAttributeFunctorArrayVisitor& operator = (const ConstAttributeFunctorArrayVisitor&) { return *this; }

        Drawable::ConstAttributeFunctor&    _af;
        Drawable::AttributeType             _type;
};

void GeometryNew::accept(ConstAttributeFunctor& af) const
{
    ConstAttributeFunctorArrayVisitor afav(af);

    if (_vertexArray.valid())
    {
        afav.applyArray(VERTICES,_vertexArray.get());
    }
    else if (_vertexAttribList.size()>0)
    {
        OSG_INFO<<"GeometryNew::accept(ConstAttributeFunctor& af): Using vertex attribute instead"<<std::endl;
        afav.applyArray(VERTICES,_vertexAttribList[0].get());
    }

    afav.applyArray(NORMALS,_normalArray.get());
    afav.applyArray(COLORS,_colorArray.get());
    afav.applyArray(SECONDARY_COLORS,_secondaryColorArray.get());
    afav.applyArray(FOG_COORDS,_fogCoordArray.get());

    for(unsigned unit=0;unit<_texCoordList.size();++unit)
    {
        afav.applyArray((AttributeType)(TEXTURE_COORDS_0+unit),_texCoordList[unit].get());
    }

    for(unsigned int index=0; index<_vertexAttribList.size(); ++index)
    {
        afav.applyArray(index,_vertexAttribList[index].get());
    }
}

void GeometryNew::accept(PrimitiveFunctor& functor) const
{
    const osg::Array* vertices = _vertexArray.get();

    if (!vertices && _vertexAttribList.size()>0)
    {
        OSG_INFO<<"Using vertex attribute instead"<<std::endl;
        vertices = _vertexAttribList[0].get();
    }

    if (!vertices || vertices->getNumElements()==0) return;

    switch(vertices->getType())
    {
    case(Array::Vec2ArrayType):
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec2*>(vertices->getDataPointer()));
        break;
    case(Array::Vec3ArrayType):
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec3*>(vertices->getDataPointer()));
        break;
    case(Array::Vec4ArrayType):
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec4*>(vertices->getDataPointer()));
        break;
    case(Array::Vec2dArrayType):
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec2d*>(vertices->getDataPointer()));
        break;
    case(Array::Vec3dArrayType):
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec3d*>(vertices->getDataPointer()));
        break;
    case(Array::Vec4dArrayType):
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec4d*>(vertices->getDataPointer()));
        break;
    default:
        OSG_WARN<<"Warning: GeometryNew::accept(PrimitiveFunctor&) cannot handle Vertex Array type"<<vertices->getType()<<std::endl;
        return;
    }

    for(PrimitiveSetList::const_iterator itr=_primitives.begin();
        itr!=_primitives.end();
        ++itr)
    {
        (*itr)->accept(functor);
    }
}

void GeometryNew::accept(PrimitiveIndexFunctor& functor) const
{
    const osg::Array* vertices = _vertexArray.get();

    if (!vertices && _vertexAttribList.size()>0)
    {
        OSG_INFO<<"GeometryNew::accept(PrimitiveIndexFunctor& functor): Using vertex attribute instead"<<std::endl;
        vertices = _vertexAttribList[0].get();
    }

    if (!vertices || vertices->getNumElements()==0) return;

    switch(vertices->getType())
    {
    case(Array::Vec2ArrayType):
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec2*>(vertices->getDataPointer()));
        break;
    case(Array::Vec3ArrayType):
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec3*>(vertices->getDataPointer()));
        break;
    case(Array::Vec4ArrayType):
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec4*>(vertices->getDataPointer()));
        break;
    case(Array::Vec2dArrayType):
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec2d*>(vertices->getDataPointer()));
        break;
    case(Array::Vec3dArrayType):
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec3d*>(vertices->getDataPointer()));
        break;
    case(Array::Vec4dArrayType):
        functor.setVertexArray(vertices->getNumElements(),static_cast<const Vec4d*>(vertices->getDataPointer()));
        break;
    default:
        OSG_WARN<<"Warning: GeometryNew::accept(PrimitiveIndexFunctor&) cannot handle Vertex Array type"<<vertices->getType()<<std::endl;
        return;
    }

    for(PrimitiveSetList::const_iterator itr=_primitives.begin();
        itr!=_primitives.end();
        ++itr)
    {
        (*itr)->accept(functor);
    }
}

GeometryNew* osg::createTexturedQuadGeometryNew(const Vec3& corner,const Vec3& widthVec,const Vec3& heightVec, float l, float b, float r, float t)
{
    GeometryNew* geom = new GeometryNew;

    Vec3Array* coords = new Vec3Array(4);
    (*coords)[0] = corner+heightVec;
    (*coords)[1] = corner;
    (*coords)[2] = corner+widthVec;
    (*coords)[3] = corner+widthVec+heightVec;
    geom->setVertexArray(coords);

    Vec2Array* tcoords = new Vec2Array(4);
    (*tcoords)[0].set(l,t);
    (*tcoords)[1].set(l,b);
    (*tcoords)[2].set(r,b);
    (*tcoords)[3].set(r,t);
    geom->setTexCoordArray(0,tcoords);

    osg::Vec4Array* colours = new osg::Vec4Array(1);
    (*colours)[0].set(1.0f,1.0f,1.0,1.0f);
    geom->setColorArray(colours);
    geom->setColorBinding(GeometryNew::BIND_OVERALL);

    osg::Vec3Array* normals = new osg::Vec3Array(1);
    (*normals)[0] = widthVec^heightVec;
    (*normals)[0].normalize();
    geom->setNormalArray(normals);
    geom->setNormalBinding(GeometryNew::BIND_OVERALL);

#if defined(OSG_GLES1_AVAILABLE) || !defined(OSG_GLES2_AVAILABLE)
    DrawElementsUByte* elems = new DrawElementsUByte(PrimitiveSet::TRIANGLES);
    elems->push_back(0);
    elems->push_back(1);
    elems->push_back(2);
    
    elems->push_back(2);
    elems->push_back(3);
    elems->push_back(0);
    geom->addPrimitiveSet(elems);
#else
    geom->addPrimitiveSet(new DrawArrays(PrimitiveSet::QUADS,0,4));
#endif

    return geom;
}