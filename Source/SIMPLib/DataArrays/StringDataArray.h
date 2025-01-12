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

#pragma once

#include <memory>
#include <string>
#include <vector>

#include <QtCore/QString>
#include <QtCore/QTextStream>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/DataArrays/IDataArray.h"

/**
 * @class StringDataArray StringDataArray.h DREAM3DLib/Common/StringDataArray.h
 * @brief Stores an array of QString objects
 *
 * @date Nov 13, 2012
 * @version 1.0
 */
class SIMPLib_EXPORT StringDataArray : public IDataArray
{
  // clang-format off
  // Start Python bindings declarations
  PYB11_BEGIN_BINDINGS(StringDataArray SUPERCLASS IDataArray)
  PYB11_SHARED_POINTERS(StringDataArray)
  PYB11_STATIC_NEW_MACRO(StringDataArray)
  PYB11_STATIC_CREATION(CreateArray OVERLOAD size_t QString bool)
  PYB11_STATIC_CREATION(CreateArray OVERLOAD size_t std::vector<size_t> QString bool)
  PYB11_PROPERTY(QString Name READ getName WRITE setName)
  PYB11_METHOD(QString getValue ARGS size_t,i)
  PYB11_METHOD(void setValue ARGS size_t,i const.QString.&,value)
  PYB11_METHOD(size_t getSize)
  PYB11_METHOD(size_t getNumberOfTuples)
  PYB11_END_BINDINGS()
  // End Python bindings declarations
  // clang-format on

public:
  using Self = StringDataArray;
  using Pointer = std::shared_ptr<Self>;
  using ConstPointer = std::shared_ptr<const Self>;
  using WeakPointer = std::weak_ptr<Self>;
  using ConstWeakPointer = std::weak_ptr<const Self>;
  static Pointer NullPointer();

  static Pointer New();

  using value_type = QString;

  /**
   * @brief Returns the name of the class for StringDataArray
   */
  QString getNameOfClass() const override;
  /**
   * @brief Returns the name of the class for StringDataArray
   */
  static QString ClassName();

  int getClassVersion() const override;

  /**
   * @brief CreateArray
   * @param numTuples
   * @param name
   * @param allocate
   * @return
   */
  static Pointer CreateArray(size_t numTuples, const QString& name, bool allocate = true);
  static Pointer CreateArray(size_t numTuples, const std::string& name, bool allocate = true);
  /**
   * @brief CreateArray
   * @param numTuples
   * @param compDims
   * @param name
   * @param allocate
   * @return
   */
  static Pointer CreateArray(size_t numTuples, const std::vector<size_t>& compDims, const QString& name, bool allocate = true);
  /**
   * @brief createNewArray
   * @param numElements
   * @param numComponents NOT USED. It is always 1.
   * @param name
   * @return
   */
  IDataArrayShPtrType createNewArray(size_t numElements, int rank, const size_t* dims, const QString& name, bool allocate = true) const override;

  /**
   * @brief createNewArray
   * @param numElements
   * @param dims
   * @param name
   * @param allocate
   * @return
   */
  IDataArray::Pointer createNewArray(size_t numElements, const std::vector<size_t>& dims, const QString& name, bool allocate = true) const override;

  /**
   * @brief ~StringDataArray
   */
  ~StringDataArray() override;

  /**
   * @brief isAllocated
   * @return
   */
  bool isAllocated() const override;

  /**
   * @brief Gives this array a human readable name
   * @param name The name of this array
   */
  virtual void setInitValue(const std::string& initValue);

  /**
   * @brief Gives this array a human readable name
   * @param name The name of this array
   */
  virtual void setInitValue(const QString& initValue);

  /**
   * @brief GetTypeName Returns a string representation of the type of data that is stored by this class. This
   * can be a primitive like char, float, int or the name of a class.
   * @return
   */
  void getXdmfTypeAndSize(QString& xdmfTypeName, int& precision) const override;
  /**
   * @brief getTypeAsString
   * @return
   */
  QString getTypeAsString() const override;

  /**
   * @brief
   */
  void takeOwnership() override;

  /**
   * @brief
   */
  void releaseOwnership() override;
  /**
   * @brief Returns a void pointer pointing to the index of the array. nullptr
   * pointers are entirely possible. No checks are performed to make sure
   * the index is with in the range of the internal data array.
   * @param i The index to have the returned pointer pointing to.
   * @return Void Pointer. Possibly nullptr.
   */
  void* getVoidPointer(size_t i) override;

  /**
   * @brief Returns the number of Tuples in the array.
   */
  size_t getNumberOfTuples() const override;

  /**
   * @brief Return the number of elements in the array
   * @return
   */
  size_t getSize() const override;

  int getNumberOfComponents() const override;

  std::vector<size_t> getComponentDimensions() const override;

  // Description:
  // Set/Get the dimension (n) of the rank. Must be >= 1. Make sure that
  // this is set before allocation.
  void SetRank(int rnk);

  /**
   * @brief getRank
   * @return
   */
  int getRank() const;

  /**
   * @brief Returns the number of bytes that make up the data type.
   * 1 = char
   * 2 = 16 bit integer
   * 4 = 32 bit integer/Float
   * 8 = 64 bit integer/Double
   */
  size_t getTypeSize() const override;

  /**
   * @brief Removes Tuples from the Array. If the size of the vector is Zero nothing is done. If the size of the
   * vector is greater than or Equal to the number of Tuples then the Array is Resized to Zero. If there are
   * indices that are larger than the size of the original (before erasing operations) then an error code (-100) is
   * returned from the program.
   * @param idxs The indices to remove
   * @return error code.
   */
  int eraseTuples(const std::vector<size_t>& idxs) override;

  /**
   * @brief Copies a Tuple from one position to another.
   * @param currentPos The index of the source data
   * @param newPos The destination index to place the copied data
   * @return
   */
  int copyTuple(size_t currentPos, size_t newPos) override;

  // This line must be here, because we are overloading the copyData pure virtual function in IDataArray.
  // This is required so that other classes can call this version of copyData from the subclasses.
  using IDataArray::copyFromArray;

  /**
   * @brief copyData This method copies the number of tuples specified by the
   * totalSrcTuples value starting from the source tuple offset value in <b>sourceArray</b>
   * into the current array starting at the target destination tuple offset value.
   *
   * For example if the DataArray has 10 tuples, the source DataArray has 10 tuples,
   *  the destTupleOffset = 5, the srcTupleOffset = 5, and the totalSrcTuples = 3,
   *  then tuples 5, 6, and 7 will be copied from the source into tuples 5, 6, and 7
   * of the destination array. In psuedo code it would be the following:
   * @code
   *  destArray[5] = sourceArray[5];
   *  destArray[6] = sourceArray[6];
   *  destArray[7] = sourceArray[7];
   *  .....
   * @endcode
   * @param destTupleOffset
   * @param sourceArray
   * @return
   */
  bool copyFromArray(size_t destTupleOffset, IDataArray::ConstPointer sourceArray, size_t srcTupleOffset, size_t totalSrcTuples) override;

  /**
   * @brief Does Nothing
   * @param pos The index of the Tuple
   * @param value pointer to value
   */
  void initializeTuple(size_t pos, const void* value) override;

  /**
   * @brief Sets all the values to empty string.
   */
  void initializeWithZeros() override;

  /**
   * @brief initializeWithValue
   * @param value
   */
  virtual void initializeWithValue(const QString& value);

  /**
   * @brief initializeWithValue
   * @param value
   */
  virtual void initializeWithValue(const std::string& value);

  /**
   * @brief deepCopy
   * @param forceNoAllocate
   * @return
   */
  IDataArrayShPtrType deepCopy(bool forceNoAllocate = false) const override;

  /**
   * @brief Reseizes the internal array
   * @param size The new size of the internal array
   * @return 1 on success, 0 on failure
   */
  int32_t resizeTotalElements(size_t size) override;

  /**
   * @brief Resizes the internal array to accomondate numTuples
   * @param numTuples
   */
  void resizeTuples(size_t numTuples) override;

  /**
   * @brief Initializes this class to zero bytes freeing any data that it currently owns
   */
  virtual void initialize();

  /**
   * @brief printTuple
   * @param out
   * @param i
   * @param delimiter
   */
  void printTuple(QTextStream& out, size_t i, char delimiter = ',') const override;

  /**
   * @brief printComponent
   * @param out
   * @param i
   * @param j
   */
  void printComponent(QTextStream& out, size_t i, int j) const override;

  /**
   * @brief getFullNameOfClass
   * @return
   */
  QString getFullNameOfClass() const;

  /**
   *
   * @param parentId
   * @return
   */
  int writeH5Data(hid_t parentId, const std::vector<size_t>& tDims) const override;

  /**
   * @brief writeXdmfAttribute
   * @param out
   * @param volDims
   * @param hdfFileName
   * @param groupPath
   * @return
   */
  int writeXdmfAttribute(QTextStream& out, const int64_t* volDims, const QString& hdfFileName, const QString& groupPath, const QString& labelb) const override;

  /**
   * @brief getInfoString
   * @return Returns a formatted string that contains general infomation about
   * the instance of the object.
   */
  QString getInfoString(SIMPL::InfoStringFormat format) const override;

  /**
   * @brief Returns a ToolTipGenerator for creating HTML tooltip tables
   * with values populated to match the current DataArray.
   * @return
   */
  ToolTipGenerator getToolTipGenerator() const override;

  /**
   * @brief readH5Data
   * @param parentId
   * @return
   */
  int readH5Data(hid_t parentId) override;

  /**
   * @brief setValue
   * @param i
   * @param value
   */
  void setValue(size_t i, const QString& value);
  /**
   * @brief getValue
   * @param i
   * @return
   */
  QString getValue(size_t i) const;

protected:
  /**
   * @brief Protected Constructor
   * @param numElements The number of elements in the internal array.
   * @param takeOwnership Will the class clean up the memory. Default=true
   */
  StringDataArray(size_t numTuples, const QString& name, bool allocate = true);

  StringDataArray();

private:
  QString m_InitValue;
  std::vector<QString> m_Array;
  bool _ownsData;

public:
  StringDataArray(const StringDataArray&) = delete;            // Copy Constructor Not Implemented
  StringDataArray(StringDataArray&&) = delete;                 // Move Constructor Not Implemented
  StringDataArray& operator=(const StringDataArray&) = delete; // Copy Assignment Not Implemented
  StringDataArray& operator=(StringDataArray&&) = delete;      // Move Assignment Not Implemented
};
