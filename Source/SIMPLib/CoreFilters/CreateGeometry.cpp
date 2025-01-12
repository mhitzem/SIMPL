/* ============================================================================
 * Copyright (c) 2009-2016 BlueQuartz Software, LLC
 *
 * Redistribution and use in source and binary forms, with or without modification,
 * are permitted provided that the following conditions are met:
 *
 * Redistributions of source code must retain the above copyright notice, this
 * list of conditions and the following disclaimer.
 *
 * Redistributions in binary form must reproduce the above copyright notice, this
 * list of conditions and the following disclaimer in the documentation and/or
 * other materials provided with the distribution.
 *
 * Neither the name of BlueQuartz Software, the US Air Force, nor the names of its
 * contributors may be used to endorse or promote products derived from this software
 * without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE
 * USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 *
 * The code contained herein was partially funded by the following contracts:
 *    United States Air Force Prime Contract FA8650-07-D-5800
 *    United States Air Force Prime Contract FA8650-10-D-5210
 *    United States Prime Contract Navy N00173-07-C-2068
 *
 * ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */
#include "CreateGeometry.h"

#include <QtCore/QTextStream>

#include "SIMPLib/SIMPLibVersion.h"
#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/DataContainers/DataContainer.h"
#include "SIMPLib/DataContainers/DataContainerArray.h"
#include "SIMPLib/FilterParameters/AbstractFilterParametersReader.h"
#include "SIMPLib/FilterParameters/BooleanFilterParameter.h"
#include "SIMPLib/FilterParameters/ChoiceFilterParameter.h"
#include "SIMPLib/FilterParameters/DataArraySelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/DataContainerSelectionFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedChoicesFilterParameter.h"
#include "SIMPLib/FilterParameters/LinkedPathCreationFilterParameter.h"
#include "SIMPLib/FilterParameters/PreflightUpdatedValueFilterParameter.h"
#include "SIMPLib/Geometry/EdgeGeom.h"
#include "SIMPLib/Geometry/HexahedralGeom.h"
#include "SIMPLib/Geometry/ImageGeom.h"
#include "SIMPLib/Geometry/QuadGeom.h"
#include "SIMPLib/Geometry/RectGridGeom.h"
#include "SIMPLib/Geometry/TetrahedralGeom.h"
#include "SIMPLib/Geometry/TriangleGeom.h"
#include "SIMPLib/Geometry/VertexGeom.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CreateGeometry::CreateGeometry() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CreateGeometry::~CreateGeometry() = default;

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CreateGeometry::setupFilterParameters()
{
  FilterParameterVectorType parameters;
  {
    LinkedChoicesFilterParameter::Pointer parameter = LinkedChoicesFilterParameter::New();
    parameter->setHumanLabel("Geometry Type");
    parameter->setPropertyName("GeometryType");
    parameter->setSetterCallback(SIMPL_BIND_SETTER(CreateGeometry, this, GeometryType));
    parameter->setGetterCallback(SIMPL_BIND_GETTER(CreateGeometry, this, GeometryType));
    std::vector<QString> choices;
    choices.push_back("Image");
    choices.push_back("Rectilinear Grid");
    choices.push_back("Vertex");
    choices.push_back("Edge");
    choices.push_back("Triangle");
    choices.push_back("Quadrilateral");
    choices.push_back("Tetrahedral");
    choices.push_back("Hexahedral");
    parameter->setChoices(choices);
    std::vector<QString> linkedProps = {"Dimensions",
                                        "Origin",
                                        "Spacing",
                                        "BoxDimensions",
                                        "ImageCellAttributeMatrixName", // ImageGeom
                                        "XBoundsArrayPath",
                                        "YBoundsArrayPath",
                                        "ZBoundsArrayPath",
                                        "RectGridCellAttributeMatrixName", // RectGridGeom
                                        "SharedVertexListArrayPath0",
                                        "VertexAttributeMatrixName0", // VertexGeom
                                        "SharedVertexListArrayPath1",
                                        "SharedEdgeListArrayPath",
                                        "VertexAttributeMatrixName1",
                                        "EdgeAttributeMatrixName", // EdgeGeom
                                        "SharedVertexListArrayPath2",
                                        "SharedTriListArrayPath",
                                        "VertexAttributeMatrixName2",
                                        "FaceAttributeMatrixName0", // TriangleGeom
                                        "SharedVertexListArrayPath3",
                                        "SharedQuadListArrayPath",
                                        "VertexAttributeMatrixName3",
                                        "FaceAttributeMatrixName1", // QuadGeom
                                        "SharedVertexListArrayPath4",
                                        "SharedTetListArrayPath",
                                        "VertexAttributeMatrixName4",
                                        "TetCellAttributeMatrixName", // TetrahedralGeom
                                        "SharedVertexListArrayPath5",
                                        "SharedHexListArrayPath",
                                        "VertexAttributeMatrixName5",
                                        "HexCellAttributeMatrixName"}; // HexahedralGeom
    parameter->setLinkedProperties(linkedProps);
    parameter->setEditable(false);
    parameter->setCategory(FilterParameter::Category::Parameter);
    parameters.push_back(parameter);
  }
  parameters.push_back(SIMPL_NEW_BOOL_FP("Treat Geometry Warnings as Errors", TreatWarningsAsErrors, FilterParameter::Category::Parameter, CreateGeometry));

  {
    std::vector<QString> choices = {"Copy Arrays", "Move Arrays"};
    parameters.push_back(SIMPL_NEW_CHOICE_FP("Array Handling", ArrayHandling, FilterParameter::Category::Parameter, CreateGeometry, choices, false));
  }

  DataContainerSelectionFilterParameter::RequirementType req;
  parameters.push_back(SIMPL_NEW_DC_SELECTION_FP("Data Container", DataContainerName, FilterParameter::Category::RequiredArray, CreateGeometry, req));
  {
    parameters.push_back(SIMPL_NEW_INT_VEC3_FP("Dimensions", Dimensions, FilterParameter::Category::Parameter, CreateGeometry, 0));
    parameters.push_back(SIMPL_NEW_FLOAT_VEC3_FP("Origin", Origin, FilterParameter::Category::Parameter, CreateGeometry, 0));
    parameters.push_back(SIMPL_NEW_FLOAT_VEC3_FP("Spacing", Spacing, FilterParameter::Category::Parameter, CreateGeometry, 0));
    parameters.back()->setLegacyPropertyName("Resolution");
    parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Cell Attribute Matrix", ImageCellAttributeMatrixName, DataContainerName, FilterParameter::Category::CreatedArray, CreateGeometry, 0));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Float, 1, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("X Bounds", XBoundsArrayPath, FilterParameter::Category::RequiredArray, CreateGeometry, req, 1));
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Y Bounds", YBoundsArrayPath, FilterParameter::Category::RequiredArray, CreateGeometry, req, 1));
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Z Bounds", ZBoundsArrayPath, FilterParameter::Category::RequiredArray, CreateGeometry, req, 1));
    parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Cell Attribute Matrix", RectGridCellAttributeMatrixName, DataContainerName, FilterParameter::Category::CreatedArray, CreateGeometry, 1));

    PreflightUpdatedValueFilterParameter::Pointer param = SIMPL_NEW_PREFLIGHTUPDATEDVALUE_FP("Box Size in Length Units", BoxDimensions, FilterParameter::Category::Parameter, CreateGeometry);
    param->setReadOnly(true);
    param->setGroupIndex(0);
    parameters.push_back(param);
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Float, 3, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Vertex List", SharedVertexListArrayPath0, FilterParameter::Category::RequiredArray, CreateGeometry, req, 2));
    parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Vertex Attribute Matrix", VertexAttributeMatrixName0, DataContainerName, FilterParameter::Category::CreatedArray, CreateGeometry, 2));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Float, 3, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Shared Vertex List", SharedVertexListArrayPath1, FilterParameter::Category::RequiredArray, CreateGeometry, req, 3));
    req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt64, 2, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Edge List", SharedEdgeListArrayPath, FilterParameter::Category::RequiredArray, CreateGeometry, req, 3));
    parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Vertex Attribute Matrix", VertexAttributeMatrixName1, DataContainerName, FilterParameter::Category::CreatedArray, CreateGeometry, 3));
    parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Edge Attribute Matrix", EdgeAttributeMatrixName, DataContainerName, FilterParameter::Category::CreatedArray, CreateGeometry, 3));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Float, 3, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Shared Vertex List", SharedVertexListArrayPath2, FilterParameter::Category::RequiredArray, CreateGeometry, req, 4));
    req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt64, 3, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Triangle List", SharedTriListArrayPath, FilterParameter::Category::RequiredArray, CreateGeometry, req, 4));
    parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Vertex Attribute Matrix", VertexAttributeMatrixName2, DataContainerName, FilterParameter::Category::CreatedArray, CreateGeometry, 4));
    parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Face Attribute Matrix", FaceAttributeMatrixName0, DataContainerName, FilterParameter::Category::CreatedArray, CreateGeometry, 4));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Float, 3, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Shared Vertex List", SharedVertexListArrayPath3, FilterParameter::Category::RequiredArray, CreateGeometry, req, 5));
    req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt64, 4, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Quadrilateral List", SharedQuadListArrayPath, FilterParameter::Category::RequiredArray, CreateGeometry, req, 5));
    parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Vertex Attribute Matrix", VertexAttributeMatrixName3, DataContainerName, FilterParameter::Category::CreatedArray, CreateGeometry, 5));
    parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Face Attribute Matrix", FaceAttributeMatrixName1, DataContainerName, FilterParameter::Category::CreatedArray, CreateGeometry, 5));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Float, 3, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Shared Vertex List", SharedVertexListArrayPath4, FilterParameter::Category::RequiredArray, CreateGeometry, req, 6));
    req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt64, 4, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Tetrahedral List", SharedTetListArrayPath, FilterParameter::Category::RequiredArray, CreateGeometry, req, 6));
    parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Vertex Attribute Matrix", VertexAttributeMatrixName4, DataContainerName, FilterParameter::Category::CreatedArray, CreateGeometry, 6));
    parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Cell Attribute Matrix", TetCellAttributeMatrixName, DataContainerName, FilterParameter::Category::CreatedArray, CreateGeometry, 6));
  }
  {
    DataArraySelectionFilterParameter::RequirementType req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::Float, 3, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Shared Vertex List", SharedVertexListArrayPath5, FilterParameter::Category::RequiredArray, CreateGeometry, req, 7));
    req = DataArraySelectionFilterParameter::CreateCategoryRequirement(SIMPL::TypeNames::UInt64, 8, AttributeMatrix::Category::Any);
    parameters.push_back(SIMPL_NEW_DA_SELECTION_FP("Hexahedral List", SharedHexListArrayPath, FilterParameter::Category::RequiredArray, CreateGeometry, req, 7));
    parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Vertex Attribute Matrix", VertexAttributeMatrixName5, DataContainerName, FilterParameter::Category::CreatedArray, CreateGeometry, 7));
    parameters.push_back(SIMPL_NEW_AM_WITH_LINKED_DC_FP("Cell Attribute Matrix", HexCellAttributeMatrixName, DataContainerName, FilterParameter::Category::CreatedArray, CreateGeometry, 7));
  }
  setFilterParameters(parameters);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CreateGeometry::initialize()
{
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CreateGeometry::dataCheck()
{
  clearErrorCode();
  clearWarningCode();
  initialize();

  DataContainer::Pointer dc = getDataContainerArray()->getPrereqDataContainer(this, getDataContainerName());

  if(getErrorCode() < 0)
  {
    return;
  }

  if(dc->getGeometry())
  {
    QString ss = QObject::tr("Selected Data Container already contains a Geometry");
    setErrorCondition(-701, ss);
    return;
  }

  switch(getGeometryType())
  {
  case 0: // ImageGeom
  {
    if(getDimensions().getX() <= 0 || getDimensions().getY() <= 0 || getDimensions().getZ() <= 0)
    {
      QString ss = QObject::tr("One of the dimensions has a size less than or equal to zero; all dimensions must be positive\n"
                               "X Dimension: %1\n"
                               "Y Dimension: %2\n"
                               "Z Dimension: %3\n")
                       .arg(getDimensions().getX())
                       .arg(getDimensions().getY())
                       .arg(getDimensions().getZ());
      setErrorCondition(-390, ss);
      return;
    }

    ImageGeom::Pointer image = ImageGeom::CreateGeometry(SIMPL::Geometry::ImageGeometry);
    IntVec3Type dims = getDimensions();
    image->setDimensions(SizeVec3Type(dims[0], dims[1], dims[2]));
    image->setOrigin(getOrigin());
    image->setSpacing(getSpacing());
    dc->setGeometry(image);

    std::vector<size_t> tDims = {image->getXPoints(), image->getYPoints(), image->getZPoints()};
    DataArrayPath path = getDataContainerName();
    path.setAttributeMatrixName(getImageCellAttributeMatrixName());
    dc->createNonPrereqAttributeMatrix(this, path, tDims, AttributeMatrix::Type::Cell);
    break;
  }
  case 1: // RectGridGeom
  {
    std::vector<size_t> cDims(1, 1);

    m_XBoundsPtr = getDataContainerArray()->getPrereqArrayFromPath<FloatArrayType>(this, getXBoundsArrayPath(), cDims);
    if(m_XBoundsPtr.lock())
    {
      m_XBounds = m_XBoundsPtr.lock()->getPointer(0);
    }
    m_YBoundsPtr = getDataContainerArray()->getPrereqArrayFromPath<FloatArrayType>(this, getYBoundsArrayPath(), cDims);
    if(m_YBoundsPtr.lock())
    {
      m_YBounds = m_YBoundsPtr.lock()->getPointer(0);
    }
    m_ZBoundsPtr = getDataContainerArray()->getPrereqArrayFromPath<FloatArrayType>(this, getZBoundsArrayPath(), cDims);
    if(m_ZBoundsPtr.lock())
    {
      m_ZBounds = m_ZBoundsPtr.lock()->getPointer(0);
    }

    if(getErrorCode() < 0)
    {
      return;
    }

    if(m_XBoundsPtr.lock()->getNumberOfTuples() < 2 || m_YBoundsPtr.lock()->getNumberOfTuples() < 2 || m_ZBoundsPtr.lock()->getNumberOfTuples() < 2)
    {
      QString ss = QObject::tr("One of the bounds arrays has a size less than two; all sizes must be at least two\n"
                               "X Bounds Size: %1\n"
                               "Y Bounds Size: %2\n"
                               "Z Bounds Size: %3\n")
                       .arg(m_XBoundsPtr.lock()->getNumberOfTuples())
                       .arg(m_YBoundsPtr.lock()->getNumberOfTuples())
                       .arg(m_ZBoundsPtr.lock()->getNumberOfTuples());
      setErrorCondition(-390, ss);
      return;
    }

    RectGridGeom::Pointer rectgrid = RectGridGeom::CreateGeometry(SIMPL::Geometry::RectGridGeometry);
    if(static_cast<int>(getArrayHandling()) == k_CopyArrays)
    {
      rectgrid->setXBounds(std::static_pointer_cast<FloatArrayType>(m_XBoundsPtr.lock()->deepCopy(getInPreflight())));
      rectgrid->setYBounds(std::static_pointer_cast<FloatArrayType>(m_YBoundsPtr.lock()->deepCopy(getInPreflight())));
      rectgrid->setZBounds(std::static_pointer_cast<FloatArrayType>(m_ZBoundsPtr.lock()->deepCopy(getInPreflight())));
    }
    else
    {

      DataContainerArray::Pointer dca = getDataContainerArray();
      DataContainer::Pointer dc = dca->getDataContainer(getXBoundsArrayPath());
      AttributeMatrix::Pointer am = dc->getAttributeMatrix(getXBoundsArrayPath());

      FloatArrayType::Pointer farray = std::dynamic_pointer_cast<FloatArrayType>(am->removeAttributeArray(getXBoundsArrayPath().getDataArrayName()));
      rectgrid->setXBounds(farray);

      farray = std::dynamic_pointer_cast<FloatArrayType>(am->removeAttributeArray(getYBoundsArrayPath().getDataArrayName()));
      rectgrid->setYBounds(farray);

      farray = std::dynamic_pointer_cast<FloatArrayType>(am->removeAttributeArray(getZBoundsArrayPath().getDataArrayName()));
      rectgrid->setZBounds(farray);
    }
    rectgrid->setDimensions(SizeVec3Type(m_XBoundsPtr.lock()->getNumberOfTuples() - 1, m_YBoundsPtr.lock()->getNumberOfTuples() - 1, m_ZBoundsPtr.lock()->getNumberOfTuples() - 1));
    dc->setGeometry(rectgrid);

    std::vector<size_t> tDims = {rectgrid->getXPoints(), rectgrid->getYPoints(), rectgrid->getZPoints()};
    DataArrayPath path = getDataContainerName();
    path.setAttributeMatrixName(getRectGridCellAttributeMatrixName());
    dc->createNonPrereqAttributeMatrix(this, path, tDims, AttributeMatrix::Type::Cell);
    break;
  }
  case 2: // VertexGeom
  {
    std::vector<size_t> cDims(1, 3);

    FloatArrayType::Pointer verts = getDataContainerArray()->getPrereqArrayFromPath<FloatArrayType>(this, getSharedVertexListArrayPath0(), cDims);

    if(getErrorCode() < 0)
    {
      return;
    }

    VertexGeom::Pointer vertex = VertexGeom::NullPointer();
    if(static_cast<int>(getArrayHandling()) == k_CopyArrays)
    {
      vertex = VertexGeom::CreateGeometry(std::static_pointer_cast<FloatArrayType>(verts->deepCopy(getInPreflight())), SIMPL::Geometry::VertexGeometry);
    }
    else
    {
      AttributeMatrix::Pointer am = getDataContainerArray()->getAttributeMatrix(getSharedVertexListArrayPath0());
      if(nullptr != verts.get())
      {
        // Remove from the Attribute Matrix FIRST
        IDataArray::Pointer vertsToRemove = am->removeAttributeArray(getSharedVertexListArrayPath0().getDataArrayName());
        // Assign the Verts to the newly create VertexGeom
        vertex = VertexGeom::CreateGeometry(verts, SIMPL::Geometry::VertexGeometry);
      }
    }
    dc->setGeometry(vertex);

    std::vector<size_t> tDims(1, vertex->getNumberOfVertices());
    DataArrayPath path = getDataContainerName();
    path.setAttributeMatrixName(getVertexAttributeMatrixName0());
    dc->createNonPrereqAttributeMatrix(this, path, tDims, AttributeMatrix::Type::Vertex);
    break;
  }
  case 3: // EdgeGeom
  {
    std::vector<size_t> cDims(1, 3);

    FloatArrayType::Pointer verts = getDataContainerArray()->getPrereqArrayFromPath<FloatArrayType>(this, getSharedVertexListArrayPath1(), cDims);
    cDims[0] = 2;
    m_EdgesPtr = getDataContainerArray()->getPrereqArrayFromPath<SharedEdgeList>(this, getSharedEdgeListArrayPath(), cDims);
    if(m_EdgesPtr.lock())
    {
      m_Edges = m_EdgesPtr.lock()->getPointer(0);
    }

    if(getErrorCode() < 0)
    {
      return;
    }

    EdgeGeom::Pointer edge = EdgeGeom::NullPointer();
    if(static_cast<int>(getArrayHandling()) == k_CopyArrays)
    {
      edge = EdgeGeom::CreateGeometry(std::static_pointer_cast<SizeTArrayType>(m_EdgesPtr.lock()->deepCopy(getInPreflight())),
                                      std::static_pointer_cast<FloatArrayType>(verts->deepCopy(getInPreflight())), SIMPL::Geometry::EdgeGeometry);
    }
    else
    {
      getDataContainerArray()->getAttributeMatrix(getSharedVertexListArrayPath1())->removeAttributeArray(getSharedVertexListArrayPath1().getDataArrayName());
      getDataContainerArray()->getAttributeMatrix(getSharedEdgeListArrayPath())->removeAttributeArray(getSharedEdgeListArrayPath().getDataArrayName());
      edge = EdgeGeom::CreateGeometry(m_EdgesPtr.lock(), verts, SIMPL::Geometry::EdgeGeometry);
    }
    dc->setGeometry(edge);

    m_NumVerts = edge->getNumberOfVertices();
    std::vector<size_t> tDims(1, edge->getNumberOfVertices());
    DataArrayPath path = getDataContainerName();
    path.setAttributeMatrixName(getVertexAttributeMatrixName1());
    dc->createNonPrereqAttributeMatrix(this, path, tDims, AttributeMatrix::Type::Vertex);
    tDims[0] = edge->getNumberOfEdges();
    path.setAttributeMatrixName(getEdgeAttributeMatrixName());
    dc->createNonPrereqAttributeMatrix(this, path, tDims, AttributeMatrix::Type::Edge);
    break;
  }
  case 4: // TriangleGeom
  {
    std::vector<size_t> cDims(1, 3);

    FloatArrayType::Pointer verts = getDataContainerArray()->getPrereqArrayFromPath<FloatArrayType>(this, getSharedVertexListArrayPath2(), cDims);
    m_TrisPtr = getDataContainerArray()->getPrereqArrayFromPath<SharedTriList>(this, getSharedTriListArrayPath(), cDims);
    if(m_TrisPtr.lock())
    {
      m_Tris = m_TrisPtr.lock()->getPointer(0);
    }

    if(getErrorCode() < 0)
    {
      return;
    }

    TriangleGeom::Pointer triangle = TriangleGeom::NullPointer();
    if(static_cast<int>(getArrayHandling()) == k_CopyArrays)
    {
      triangle = TriangleGeom::CreateGeometry(std::static_pointer_cast<SharedTriList>(m_TrisPtr.lock()->deepCopy(getInPreflight())),
                                              std::static_pointer_cast<FloatArrayType>(verts->deepCopy(getInPreflight())), SIMPL::Geometry::TriangleGeometry);
    }
    else
    {
      getDataContainerArray()->getAttributeMatrix(getSharedVertexListArrayPath2())->removeAttributeArray(getSharedVertexListArrayPath2().getDataArrayName());
      getDataContainerArray()->getAttributeMatrix(getSharedTriListArrayPath())->removeAttributeArray(getSharedTriListArrayPath().getDataArrayName());
      triangle = TriangleGeom::CreateGeometry(m_TrisPtr.lock(), verts, SIMPL::Geometry::TriangleGeometry);
    }
    dc->setGeometry(triangle);

    m_NumVerts = triangle->getNumberOfVertices();
    std::vector<size_t> tDims(1, triangle->getNumberOfVertices());
    DataArrayPath path = getDataContainerName();
    path.setAttributeMatrixName(getVertexAttributeMatrixName2());
    dc->createNonPrereqAttributeMatrix(this, path, tDims, AttributeMatrix::Type::Vertex);
    tDims[0] = triangle->getNumberOfTris();
    path.setAttributeMatrixName(getFaceAttributeMatrixName0());
    dc->createNonPrereqAttributeMatrix(this, path, tDims, AttributeMatrix::Type::Face);
    break;
  }
  case 5: // QuadGeom
  {
    std::vector<size_t> cDims(1, 3);

    FloatArrayType::Pointer verts = getDataContainerArray()->getPrereqArrayFromPath<FloatArrayType>(this, getSharedVertexListArrayPath3(), cDims);
    cDims[0] = 4;
    m_QuadsPtr = getDataContainerArray()->getPrereqArrayFromPath<SharedQuadList>(this, getSharedQuadListArrayPath(), cDims);
    if(m_QuadsPtr.lock())
    {
      m_Quads = m_QuadsPtr.lock()->getPointer(0);
    }

    if(getErrorCode() < 0)
    {
      return;
    }

    QuadGeom::Pointer quadrilateral = QuadGeom::NullPointer();
    if(static_cast<int>(getArrayHandling()) == k_CopyArrays)
    {
      quadrilateral = QuadGeom::CreateGeometry(std::static_pointer_cast<SizeTArrayType>(m_QuadsPtr.lock()->deepCopy(getInPreflight())),
                                               std::static_pointer_cast<FloatArrayType>(verts->deepCopy(getInPreflight())), SIMPL::Geometry::QuadGeometry);
    }
    else
    {
      getDataContainerArray()->getAttributeMatrix(getSharedVertexListArrayPath3())->removeAttributeArray(getSharedVertexListArrayPath3().getDataArrayName());
      getDataContainerArray()->getAttributeMatrix(getSharedQuadListArrayPath())->removeAttributeArray(getSharedQuadListArrayPath().getDataArrayName());
      quadrilateral = QuadGeom::CreateGeometry(m_QuadsPtr.lock(), verts, SIMPL::Geometry::QuadGeometry);
    }
    dc->setGeometry(quadrilateral);

    m_NumVerts = quadrilateral->getNumberOfVertices();
    std::vector<size_t> tDims(1, quadrilateral->getNumberOfVertices());
    DataArrayPath path = getDataContainerName();
    path.setAttributeMatrixName(getVertexAttributeMatrixName3());
    dc->createNonPrereqAttributeMatrix(this, path, tDims, AttributeMatrix::Type::Vertex);
    tDims[0] = quadrilateral->getNumberOfQuads();
    path.setAttributeMatrixName(getFaceAttributeMatrixName1());
    dc->createNonPrereqAttributeMatrix(this, path, tDims, AttributeMatrix::Type::Face);
    break;
  }
  case 6: // TetrahedralGeom
  {
    std::vector<size_t> cDims(1, 3);

    FloatArrayType::Pointer verts = getDataContainerArray()->getPrereqArrayFromPath<FloatArrayType>(this, getSharedVertexListArrayPath4(), cDims);
    cDims[0] = 4;
    m_TetsPtr = getDataContainerArray()->getPrereqArrayFromPath<SharedTetList>(this, getSharedTetListArrayPath(), cDims);
    if(m_TetsPtr.lock())
    {
      m_Tets = m_TetsPtr.lock()->getPointer(0);
    }

    if(getErrorCode() < 0)
    {
      return;
    }

    TetrahedralGeom::Pointer tetrahedral = TetrahedralGeom::NullPointer();
    if(static_cast<int>(getArrayHandling()) == k_CopyArrays)
    {
      tetrahedral = TetrahedralGeom::CreateGeometry(std::static_pointer_cast<SizeTArrayType>(m_TetsPtr.lock()->deepCopy(getInPreflight())),
                                                    std::static_pointer_cast<FloatArrayType>(verts->deepCopy(getInPreflight())), SIMPL::Geometry::TetrahedralGeometry);
    }
    else
    {
      getDataContainerArray()->getAttributeMatrix(getSharedVertexListArrayPath4())->removeAttributeArray(getSharedVertexListArrayPath4().getDataArrayName());
      getDataContainerArray()->getAttributeMatrix(getSharedTetListArrayPath())->removeAttributeArray(getSharedTetListArrayPath().getDataArrayName());
      tetrahedral = TetrahedralGeom::CreateGeometry(m_TetsPtr.lock(), verts, SIMPL::Geometry::TetrahedralGeometry);
    }

    dc->setGeometry(tetrahedral);

    m_NumVerts = tetrahedral->getNumberOfVertices();
    std::vector<size_t> tDims(1, tetrahedral->getNumberOfVertices());
    DataArrayPath path = getDataContainerName();
    path.setAttributeMatrixName(getVertexAttributeMatrixName4());
    dc->createNonPrereqAttributeMatrix(this, path, tDims, AttributeMatrix::Type::Vertex);
    tDims[0] = tetrahedral->getNumberOfTets();
    path.setAttributeMatrixName(getTetCellAttributeMatrixName());
    dc->createNonPrereqAttributeMatrix(this, path, tDims, AttributeMatrix::Type::Cell);
    break;
  }
  case 7: // HexahedralGeom
  {
    std::vector<size_t> cDims(1, 3);

    FloatArrayType::Pointer verts = getDataContainerArray()->getPrereqArrayFromPath<FloatArrayType>(this, getSharedVertexListArrayPath5(), cDims);
    cDims[0] = 8;
    m_HexesPtr = getDataContainerArray()->getPrereqArrayFromPath<SharedHexList>(this, getSharedHexListArrayPath(), cDims);
    if(m_HexesPtr.lock())
    {
      m_Hexes = m_HexesPtr.lock()->getPointer(0);
    }

    if(getErrorCode() < 0)
    {
      return;
    }

    HexahedralGeom::Pointer hexahedral = HexahedralGeom::NullPointer();
    if(static_cast<int>(getArrayHandling()) == k_CopyArrays)
    {
      hexahedral = HexahedralGeom::CreateGeometry(std::static_pointer_cast<SizeTArrayType>(m_HexesPtr.lock()->deepCopy(getInPreflight())),
                                                  std::static_pointer_cast<FloatArrayType>(verts->deepCopy(getInPreflight())), SIMPL::Geometry::HexahedralGeometry);
    }
    else
    {
      getDataContainerArray()->getAttributeMatrix(getSharedVertexListArrayPath5())->removeAttributeArray(getSharedVertexListArrayPath5().getDataArrayName());
      getDataContainerArray()->getAttributeMatrix(getSharedHexListArrayPath())->removeAttributeArray(getSharedHexListArrayPath().getDataArrayName());
      hexahedral = HexahedralGeom::CreateGeometry(m_HexesPtr.lock(), verts, SIMPL::Geometry::HexahedralGeometry);
    }

    dc->setGeometry(hexahedral);

    m_NumVerts = hexahedral->getNumberOfVertices();
    std::vector<size_t> tDims(1, hexahedral->getNumberOfVertices());
    DataArrayPath path = getDataContainerName();
    path.setAttributeMatrixName(getVertexAttributeMatrixName5());
    dc->createNonPrereqAttributeMatrix(this, path, tDims, AttributeMatrix::Type::Vertex);
    tDims[0] = hexahedral->getNumberOfHexas();
    path.setAttributeMatrixName(getHexCellAttributeMatrixName());
    dc->createNonPrereqAttributeMatrix(this, path, tDims, AttributeMatrix::Type::Cell);
    break;
  }
  default: {
    QString ss = QObject::tr("Invalid selection for Geometry type");
    setErrorCondition(-701, ss);
    break;
  }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CreateGeometry::execute()
{
  dataCheck();
  if(getErrorCode() < 0)
  {
    return;
  }

  switch(m_GeometryType)
  {
  case 0: // ImageGeom
  {
    // Checked during preflight()
    break;
  }
  case 1: // RectGridGeom
  {
    float xval = m_XBounds[0];
    for(size_t i = 1; i < m_XBoundsPtr.lock()->getNumberOfTuples(); i++)
    {
      if(xval > m_XBounds[i])
      {
        QString ss = QObject::tr("Supplied X Bounds array is not strictly increasing; this results in negative resolutions\n"
                                 "Index %1 Value: %2\n"
                                 "Index %3 Value: %4")
                         .arg(i - 1)
                         .arg(xval)
                         .arg(i)
                         .arg(m_XBounds[i]);
        if(m_TreatWarningsAsErrors)
        {
          setErrorCondition(-1, ss);
        }
        else
        {
          setWarningCondition(-1, ss);
        }
        return;
      }
      xval = m_XBounds[i];
    }

    float yval = m_YBounds[0];
    for(size_t i = 1; i < m_YBoundsPtr.lock()->getNumberOfTuples(); i++)
    {
      if(yval > m_YBounds[i])
      {
        QString ss = QObject::tr("Supplied Y Bounds array is not strictly increasing; this results in negative resolutions\n"
                                 "Index %1 Value: %2\n"
                                 "Index %3 Value: %4")
                         .arg(i - 1)
                         .arg(yval)
                         .arg(i)
                         .arg(m_YBounds[i]);
        if(m_TreatWarningsAsErrors)
        {
          setErrorCondition(-1, ss);
        }
        else
        {
          setWarningCondition(-1, ss);
        }
        return;
      }
      yval = m_YBounds[i];
    }

    float zval = m_ZBounds[0];
    for(size_t i = 1; i < m_ZBoundsPtr.lock()->getNumberOfTuples(); i++)
    {
      if(zval > m_ZBounds[i])
      {
        QString ss = QObject::tr("Supplied Z Bounds array is not strictly increasing; this results in negative resolutions\n"
                                 "Index %1 Value: %2\n"
                                 "Index %3 Value: %4")
                         .arg(i - 1)
                         .arg(zval)
                         .arg(i)
                         .arg(m_ZBounds[i]);
        if(m_TreatWarningsAsErrors)
        {
          setErrorCondition(-1, ss);
        }
        else
        {
          setWarningCondition(-1, ss);
        }
        return;
      }
      zval = m_ZBounds[i];
    }

    break;
  }
  case 2: // VertexGeom
  {
    // Checked during preflight()
    break;
  }
  case 3: // EdgeGeom
  {
    size_t idx = 0;
    for(size_t i = 0; i < m_EdgesPtr.lock()->getSize(); i++)
    {
      if(m_Edges[i] > idx)
      {
        idx = m_Edges[i];
      }
    }

    if((idx + 1) > m_NumVerts)
    {
      QString ss = QObject::tr("Supplied edge list contains a vertex index larger than the total length of the supplied shared vertex list\n"
                               "Index Value: %1\n"
                               "Number of Vertices: %2")
                       .arg(idx)
                       .arg(m_NumVerts);
      if(m_TreatWarningsAsErrors)
      {
        setErrorCondition(-1, ss);
      }
      else
      {
        setWarningCondition(-1, ss);
      }
      return;
    }

    break;
  }
  case 4: // TriangleGeom
  {
    size_t idx = 0;
    for(size_t i = 0; i < m_TrisPtr.lock()->getSize(); i++)
    {
      if(m_Tris[i] > idx)
      {
        idx = m_Tris[i];
      }
    }

    if((idx + 1) > m_NumVerts)
    {
      QString ss = QObject::tr("Supplied triangle list contains a vertex index larger than the total length of the supplied shared vertex list\n"
                               "Index Value: %1\n"
                               "Number of Vertices: %2")
                       .arg(idx)
                       .arg(m_NumVerts);
      if(m_TreatWarningsAsErrors)
      {
        setErrorCondition(-1, ss);
      }
      else
      {
        setWarningCondition(-1, ss);
      }
      return;
    }

    break;
  }
  case 5: // QuadGeom
  {
    size_t idx = 0;
    for(size_t i = 0; i < m_QuadsPtr.lock()->getSize(); i++)
    {
      if(m_Quads[i] > idx)
      {
        idx = m_Quads[i];
      }
    }

    if((idx + 1) > m_NumVerts)
    {
      QString ss = QObject::tr("Supplied quadrilateral list contains a vertex index larger than the total length of the supplied shared vertex list\n"
                               "Index Value: %1\n"
                               "Number of Vertices: %2")
                       .arg(idx)
                       .arg(m_NumVerts);
      if(m_TreatWarningsAsErrors)
      {
        setErrorCondition(-1, ss);
      }
      else
      {
        setWarningCondition(-1, ss);
      }
      return;
    }

    break;
  }
  case 6: // TetrahedralGeom
  {
    size_t idx = 0;
    for(size_t i = 0; i < m_TetsPtr.lock()->getSize(); i++)
    {
      if(m_Tets[i] > idx)
      {
        idx = m_Tets[i];
      }
    }

    if((idx + 1) > m_NumVerts)
    {
      QString ss = QObject::tr("Supplied tetrahedra list contains a vertex index larger than the total length of the supplied shared vertex list\n"
                               "Index Value: %1\n"
                               "Number of Vertices: %2")
                       .arg(idx)
                       .arg(m_NumVerts);
      if(m_TreatWarningsAsErrors)
      {
        setErrorCondition(-1, ss);
      }
      else
      {
        setWarningCondition(-1, ss);
      }
      return;
    }

    break;
  }
  case 7: // HexahedralGeom
  {
    size_t idx = 0;
    for(size_t i = 0; i < m_HexesPtr.lock()->getSize(); i++)
    {
      if(m_Hexes[i] > idx)
      {
        idx = m_Hexes[i];
      }
    }

    if((idx + 1) > m_NumVerts)
    {
      QString ss = QObject::tr("Supplied hexahedra list contains a vertex index larger than the total length of the supplied shared vertex list\n"
                               "Index Value: %1\n"
                               "Number of Vertices: %2")
                       .arg(idx)
                       .arg(m_NumVerts);
      if(m_TreatWarningsAsErrors)
      {
        setErrorCondition(-1, ss);
      }
      else
      {
        setWarningCondition(-1, ss);
      }
      return;
    }

    break;
  }
  default: {
    break;
  }
  }
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CreateGeometry::getBoxDimensions()
{
  QString desc;
  QTextStream ss(&desc);
  float halfRes[3] = {m_Spacing[0] * 0.5f, m_Spacing[1] * 0.5f, m_Spacing[2] * 0.5f};
  ss << "Extents:\n"
     << "X Extent: 0 to " << m_Dimensions[0] - 1 << " (dimension: " << m_Dimensions[0] << ")\n"
     << "Y Extent: 0 to " << m_Dimensions[1] - 1 << " (dimension: " << m_Dimensions[1] << ")\n"
     << "Z Extent: 0 to " << m_Dimensions[2] - 1 << " (dimension: " << m_Dimensions[2] << ")\n"
     << "Bounds:\n"
     << "X Range: " << (m_Origin[0] - halfRes[0]) << " to " << (m_Origin[0] - halfRes[0] + m_Dimensions[0] * m_Spacing[0]) << " (delta: " << (m_Dimensions[0] * m_Spacing[0]) << ")\n"
     << "Y Range: " << (m_Origin[1] - halfRes[1]) << " to " << (m_Origin[1] - halfRes[1] + m_Dimensions[1] * m_Spacing[1]) << " (delta: " << (m_Dimensions[1] * m_Spacing[1]) << ")\n"
     << "Z Range: " << (m_Origin[2] - halfRes[2]) << " to " << (m_Origin[2] - halfRes[2] + m_Dimensions[2] * m_Spacing[2]) << " (delta: " << (m_Dimensions[2] * m_Spacing[2]) << ")\n";
  return desc;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
AbstractFilter::Pointer CreateGeometry::newFilterInstance(bool copyFilterParameters) const
{
  CreateGeometry::Pointer filter = CreateGeometry::New();
  if(copyFilterParameters)
  {
    copyFilterParameterInstanceVariables(filter.get());
  }
  return filter;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CreateGeometry::getCompiledLibraryName() const
{
  return Core::CoreBaseName;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CreateGeometry::getBrandingString() const
{
  return "SIMPLib Core Filter";
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CreateGeometry::getFilterVersion() const
{
  QString version;
  QTextStream vStream(&version);
  vStream << SIMPLib::Version::Major() << "." << SIMPLib::Version::Minor() << "." << SIMPLib::Version::Patch();
  return version;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CreateGeometry::getGroupName() const
{
  return SIMPL::FilterGroups::CoreFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QUuid CreateGeometry::getUuid() const
{
  return QUuid("{9ac220b9-14f9-581a-9bac-5714467589cc}");
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CreateGeometry::getSubGroupName() const
{
  return SIMPL::FilterSubGroups::GeometryFilters;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
QString CreateGeometry::getHumanLabel() const
{
  return "Create Geometry";
}

// -----------------------------------------------------------------------------
CreateGeometry::Pointer CreateGeometry::NullPointer()
{
  return Pointer(static_cast<Self*>(nullptr));
}

// -----------------------------------------------------------------------------
std::shared_ptr<CreateGeometry> CreateGeometry::New()
{
  struct make_shared_enabler : public CreateGeometry
  {
  };
  std::shared_ptr<make_shared_enabler> val = std::make_shared<make_shared_enabler>();
  val->setupFilterParameters();
  return val;
}

// -----------------------------------------------------------------------------
QString CreateGeometry::getNameOfClass() const
{
  return QString("CreateGeometry");
}

// -----------------------------------------------------------------------------
QString CreateGeometry::ClassName()
{
  return QString("CreateGeometry");
}

// -----------------------------------------------------------------------------
void CreateGeometry::setGeometryType(int value)
{
  m_GeometryType = value;
}

// -----------------------------------------------------------------------------
int CreateGeometry::getGeometryType() const
{
  return m_GeometryType;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setDataContainerName(const DataArrayPath& value)
{
  m_DataContainerName = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getDataContainerName() const
{
  return m_DataContainerName;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setSharedVertexListArrayPath0(const DataArrayPath& value)
{
  m_SharedVertexListArrayPath0 = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getSharedVertexListArrayPath0() const
{
  return m_SharedVertexListArrayPath0;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setSharedVertexListArrayPath1(const DataArrayPath& value)
{
  m_SharedVertexListArrayPath1 = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getSharedVertexListArrayPath1() const
{
  return m_SharedVertexListArrayPath1;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setSharedVertexListArrayPath2(const DataArrayPath& value)
{
  m_SharedVertexListArrayPath2 = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getSharedVertexListArrayPath2() const
{
  return m_SharedVertexListArrayPath2;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setSharedVertexListArrayPath3(const DataArrayPath& value)
{
  m_SharedVertexListArrayPath3 = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getSharedVertexListArrayPath3() const
{
  return m_SharedVertexListArrayPath3;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setSharedVertexListArrayPath4(const DataArrayPath& value)
{
  m_SharedVertexListArrayPath4 = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getSharedVertexListArrayPath4() const
{
  return m_SharedVertexListArrayPath4;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setSharedVertexListArrayPath5(const DataArrayPath& value)
{
  m_SharedVertexListArrayPath5 = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getSharedVertexListArrayPath5() const
{
  return m_SharedVertexListArrayPath5;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setSharedEdgeListArrayPath(const DataArrayPath& value)
{
  m_SharedEdgeListArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getSharedEdgeListArrayPath() const
{
  return m_SharedEdgeListArrayPath;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setSharedTriListArrayPath(const DataArrayPath& value)
{
  m_SharedTriListArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getSharedTriListArrayPath() const
{
  return m_SharedTriListArrayPath;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setSharedQuadListArrayPath(const DataArrayPath& value)
{
  m_SharedQuadListArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getSharedQuadListArrayPath() const
{
  return m_SharedQuadListArrayPath;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setSharedTetListArrayPath(const DataArrayPath& value)
{
  m_SharedTetListArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getSharedTetListArrayPath() const
{
  return m_SharedTetListArrayPath;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setSharedHexListArrayPath(const DataArrayPath& value)
{
  m_SharedHexListArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getSharedHexListArrayPath() const
{
  return m_SharedHexListArrayPath;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setXBoundsArrayPath(const DataArrayPath& value)
{
  m_XBoundsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getXBoundsArrayPath() const
{
  return m_XBoundsArrayPath;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setYBoundsArrayPath(const DataArrayPath& value)
{
  m_YBoundsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getYBoundsArrayPath() const
{
  return m_YBoundsArrayPath;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setZBoundsArrayPath(const DataArrayPath& value)
{
  m_ZBoundsArrayPath = value;
}

// -----------------------------------------------------------------------------
DataArrayPath CreateGeometry::getZBoundsArrayPath() const
{
  return m_ZBoundsArrayPath;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setDimensions(const IntVec3Type& value)
{
  m_Dimensions = value;
}

// -----------------------------------------------------------------------------
IntVec3Type CreateGeometry::getDimensions() const
{
  return m_Dimensions;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setOrigin(const FloatVec3Type& value)
{
  m_Origin = value;
}

// -----------------------------------------------------------------------------
FloatVec3Type CreateGeometry::getOrigin() const
{
  return m_Origin;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setSpacing(const FloatVec3Type& value)
{
  m_Spacing = value;
}

// -----------------------------------------------------------------------------
FloatVec3Type CreateGeometry::getSpacing() const
{
  return m_Spacing;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setImageCellAttributeMatrixName(const QString& value)
{
  m_ImageCellAttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
QString CreateGeometry::getImageCellAttributeMatrixName() const
{
  return m_ImageCellAttributeMatrixName;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setRectGridCellAttributeMatrixName(const QString& value)
{
  m_RectGridCellAttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
QString CreateGeometry::getRectGridCellAttributeMatrixName() const
{
  return m_RectGridCellAttributeMatrixName;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setVertexAttributeMatrixName0(const QString& value)
{
  m_VertexAttributeMatrixName0 = value;
}

// -----------------------------------------------------------------------------
QString CreateGeometry::getVertexAttributeMatrixName0() const
{
  return m_VertexAttributeMatrixName0;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setVertexAttributeMatrixName1(const QString& value)
{
  m_VertexAttributeMatrixName1 = value;
}

// -----------------------------------------------------------------------------
QString CreateGeometry::getVertexAttributeMatrixName1() const
{
  return m_VertexAttributeMatrixName1;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setVertexAttributeMatrixName2(const QString& value)
{
  m_VertexAttributeMatrixName2 = value;
}

// -----------------------------------------------------------------------------
QString CreateGeometry::getVertexAttributeMatrixName2() const
{
  return m_VertexAttributeMatrixName2;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setVertexAttributeMatrixName3(const QString& value)
{
  m_VertexAttributeMatrixName3 = value;
}

// -----------------------------------------------------------------------------
QString CreateGeometry::getVertexAttributeMatrixName3() const
{
  return m_VertexAttributeMatrixName3;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setVertexAttributeMatrixName4(const QString& value)
{
  m_VertexAttributeMatrixName4 = value;
}

// -----------------------------------------------------------------------------
QString CreateGeometry::getVertexAttributeMatrixName4() const
{
  return m_VertexAttributeMatrixName4;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setVertexAttributeMatrixName5(const QString& value)
{
  m_VertexAttributeMatrixName5 = value;
}

// -----------------------------------------------------------------------------
QString CreateGeometry::getVertexAttributeMatrixName5() const
{
  return m_VertexAttributeMatrixName5;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setEdgeAttributeMatrixName(const QString& value)
{
  m_EdgeAttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
QString CreateGeometry::getEdgeAttributeMatrixName() const
{
  return m_EdgeAttributeMatrixName;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setFaceAttributeMatrixName0(const QString& value)
{
  m_FaceAttributeMatrixName0 = value;
}

// -----------------------------------------------------------------------------
QString CreateGeometry::getFaceAttributeMatrixName0() const
{
  return m_FaceAttributeMatrixName0;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setFaceAttributeMatrixName1(const QString& value)
{
  m_FaceAttributeMatrixName1 = value;
}

// -----------------------------------------------------------------------------
QString CreateGeometry::getFaceAttributeMatrixName1() const
{
  return m_FaceAttributeMatrixName1;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setTetCellAttributeMatrixName(const QString& value)
{
  m_TetCellAttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
QString CreateGeometry::getTetCellAttributeMatrixName() const
{
  return m_TetCellAttributeMatrixName;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setHexCellAttributeMatrixName(const QString& value)
{
  m_HexCellAttributeMatrixName = value;
}

// -----------------------------------------------------------------------------
QString CreateGeometry::getHexCellAttributeMatrixName() const
{
  return m_HexCellAttributeMatrixName;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setTreatWarningsAsErrors(bool value)
{
  m_TreatWarningsAsErrors = value;
}

// -----------------------------------------------------------------------------
bool CreateGeometry::getTreatWarningsAsErrors() const
{
  return m_TreatWarningsAsErrors;
}

// -----------------------------------------------------------------------------
void CreateGeometry::setArrayHandling(bool value)
{
  m_ArrayHandling = value;
}

// -----------------------------------------------------------------------------
bool CreateGeometry::getArrayHandling() const
{
  return m_ArrayHandling;
}
