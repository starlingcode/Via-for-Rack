
# Via for Rack

Source for the Rack plugins of the Via modules (https://starling.space/via)

User documentation can be found here: https://starling.space/via/platform-info#rack

## Build Instructions

First, build Rack (https://vcvrack.com/manual/Building.html)

Change the current directory to Rack/plugins and clone the repo:
```
git clone https://github.com/starlingcode/Via-for-Rack.git
```
Navigate to the Via plugin directory:
```
cd Via-for-Rack
```
Update the submodules:
```
git submodule update --init --recursive
```
Then, build the plugins:
```
make
```

## Contributing

We welcome Issues and Pull Requests to this repository if you have suggestions for improvement.

