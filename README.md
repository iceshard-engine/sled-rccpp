# Wrapper library around RuntimeCompiled C++ library

This library aims at easing the use of RCC++.

Right now the following features are included:
* Removal of the IObject interface being a base of your own types.
* Ability to define a custom constructor to be used for runtime-recompilation.
* Ability to define a static `on_reconstruct` method that creates the type with a placement new and allows to use RAII types.
* Helper type 'SlSwappablePtr' that tracks the current 'owner' _(ex.: a unique_ptr-like type)_ of the runtime-compilable object and updates the pointer if a compilation succeeded.
  * Allows to ommit the 'Listener' interface from the original Library and pointers are updated out-of-the box always!
 
## Planned features
* Memory transfering _(reuse the same allocated memory == no pointer updates)_
* Additional improvements in how types are reconstructed and serialized.
* Simplification of project setups based on build information from the workspace.
  * Initially it will target only **Iceshards** build system.
