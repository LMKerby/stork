#include "SerpentcoupleriiApp.h"
#include "Moose.h"
#include "AppFactory.h"
#include "ModulesApp.h"
#include "MooseSyntax.h"
#include "RunSerpent.h"
#include "HeatToMoose.h"
#include "ElementTransfer.h"
#include "ElementHeatSource.h"


template<>
InputParameters validParams<SerpentcoupleriiApp>()
{
  InputParameters params = validParams<MooseApp>();

  params.set<bool>("use_legacy_uo_initialization") = false;
  params.set<bool>("use_legacy_uo_aux_computation") = false;
  params.set<bool>("use_legacy_output_syntax") = false;

  return params;
}

SerpentcoupleriiApp::SerpentcoupleriiApp(InputParameters parameters) :
    MooseApp(parameters)
{
  Moose::registerObjects(_factory);
  ModulesApp::registerObjects(_factory);
  SerpentcoupleriiApp::registerObjects(_factory);

  Moose::associateSyntax(_syntax, _action_factory);
  ModulesApp::associateSyntax(_syntax, _action_factory);
  SerpentcoupleriiApp::associateSyntax(_syntax, _action_factory);
}

SerpentcoupleriiApp::~SerpentcoupleriiApp()
{
}

// External entry point for dynamic application loading
extern "C" void SerpentcoupleriiApp__registerApps() { SerpentcoupleriiApp::registerApps(); }
void
SerpentcoupleriiApp::registerApps()
{
  registerApp(SerpentcoupleriiApp);
}

// External entry point for dynamic object registration
extern "C" void SerpentcoupleriiApp__registerObjects(Factory & factory) { SerpentcoupleriiApp::registerObjects(factory); }
void
SerpentcoupleriiApp::registerObjects(Factory & factory)
{
  registerUserObject(ElementTransfer);
  registerUserObject(RunSerpent);
  registerUserObject(HeatToMoose);
  registerKernel(ElementHeatSource);

}

// External entry point for dynamic syntax association
extern "C" void SerpentcoupleriiApp__associateSyntax(Syntax & syntax, ActionFactory & action_factory) { SerpentcoupleriiApp::associateSyntax(syntax, action_factory); }
void
SerpentcoupleriiApp::associateSyntax(Syntax & /*syntax*/, ActionFactory & /*action_factory*/)
{
}
