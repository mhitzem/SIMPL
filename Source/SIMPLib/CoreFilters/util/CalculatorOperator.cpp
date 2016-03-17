/* ============================================================================
* Copyright (c) 2009-2015 BlueQuartz Software, LLC
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
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include "CalculatorOperator.h"

#include "SIMPLib/Math/SIMPLibMath.h"

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CalculatorOperator::CalculatorOperator() :
  CalculatorItem(),
  m_Precedence(Unknown_Precedence)
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CalculatorOperator::~CalculatorOperator()
{

}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool CalculatorOperator::hasHigherPrecedence(const CalculatorOperator::Pointer other)
{
  if (m_Precedence > other->m_Precedence)
  {
    return true;
  }

  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double CalculatorOperator::calculate(AbstractFilter* filter, const QString &newArrayName, QStack<ICalculatorArray::Pointer> &executionStack, int index)
{
  // This should never be executed
  return 0.0;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
bool CalculatorOperator::checkValidity(QVector<CalculatorItem::Pointer> infixVector, int currentIndex)
{
  // This should never be executed
  return false;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CalculatorOperator::Precedence CalculatorOperator::getPrecedence()
{
  return m_Precedence;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorOperator::setPrecedence(Precedence precedence)
{
  m_Precedence = precedence;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
CalculatorOperator::OperatorType CalculatorOperator::getOperatorType()
{
  return m_OperatorType;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void CalculatorOperator::setOperatorType(OperatorType type)
{
  m_OperatorType = type;
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double CalculatorOperator::toDegrees(double radians)
{
  return radians * (180.0 / M_PI);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double CalculatorOperator::toRadians(double degrees)
{
  return degrees * (M_PI / 180.0);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
double CalculatorOperator::root(double base, double root)
{
  if (root == 0)
  {
    return std::numeric_limits<double>().infinity();
  }

  return pow(base, 1 / root);
}

