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
* The code contained herein was partially funded by the followig contracts:
*    United States Air Force Prime Contract FA8650-07-D-5800
*    United States Air Force Prime Contract FA8650-10-D-5210
*    United States Prime Contract Navy N00173-07-C-2068
*
* ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ */

#include <QtCore/QCoreApplication>
#include <QtCore/QMetaObject>

#include "SIMPLib/SIMPLib.h"
#include "SIMPLib/SIMPLibVersion.h"
#include "SIMPLib/Math/SIMPLibMath.h"
#include "SIMPLib/Common/Constants.h"
#include "SIMPLib/Common/FilterManager.h"
#include "SIMPLib/Common/FilterFactory.hpp"
#include "SIMPLib/Common/FilterPipeline.h"
#include "SIMPLib/Common/TemplateHelpers.hpp"
#include "SIMPLib/Plugin/ISIMPLibPlugin.h"
#include "SIMPLib/Plugin/SIMPLibPluginLoader.h"
#include "SIMPLib/Utilities/UnitTestSupport.hpp"
#include "SIMPLib/Utilities/QMetaObjectUtilities.h"

#include "SIMPLib/CoreFilters/Breakpoint.h"
#include "SIMPLib/TestFilters/ChangeGlobalValue.h"

#include "SIMPLTestFileLocations.h"

int GlobalVariable = 0;

class PipelinePauseTest
{
public:
  PipelinePauseTest() {}
  virtual ~PipelinePauseTest() {}
  SIMPL_TYPE_MACRO(PipelinePauseTest)

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  FilterPipeline::Pointer createPipeline()
  {
    FilterPipeline::Pointer pipeline = FilterPipeline::New();
    {
      ChangeGlobalValue::Pointer filter = ChangeGlobalValue::New();
      filter->setValue(3);
      filter->setGlobalValue(&GlobalVariable);
      pipeline->pushBack(filter);
    }
    {
      Breakpoint::Pointer filter = Breakpoint::New();
      pipeline->pushBack(filter);
    }
    {
      ChangeGlobalValue::Pointer filter = ChangeGlobalValue::New();
      filter->setValue(7);
      filter->setGlobalValue(&GlobalVariable);
      pipeline->pushBack(filter);
    }

    return pipeline;
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void PauseTest()
  {
    QVariant var;
    bool propWasSet = false;

    FilterPipeline::Pointer pipeline = createPipeline();

    pipeline->execute();

    DREAM3D_REQUIRE_EQUAL(GlobalVariable, 7);
  }

  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void operator()()
  {
    std::cout << "#### PipelinePauseTest Starting ####" << std::endl;
    int err = EXIT_SUCCESS;

    DREAM3D_REGISTER_TEST(PauseTest())
  }

public slots:
  // -----------------------------------------------------------------------------
  //
  // -----------------------------------------------------------------------------
  void PipelinePaused()
  {

  }

private:
  PipelinePauseTest(const PipelinePauseTest&); // Copy Constructor Not Implemented
  void operator=(const PipelinePauseTest&); // Operator '=' Not Implemented
};

Q_DECLARE_METATYPE(int*);

