# BLEND

[![CI](https://github.com/cascadiaquakes/blend/actions/workflows/ci.yml/badge.svg)](https://github.com/cascadiaquakes/blend/actions/workflows/ci.yml)
[![Documentation](https://github.com/cascadiaquakes/blend/actions/workflows/pages.yml/badge.svg)](https://github.com/cascadiaquakes/blend/actions/workflows/pages.yml)
[![Documentation](https://img.shields.io/badge/docs-latest-green.svg)](https://ajalalab.com/blend/)
[![GitHub release](https://img.shields.io/github/v/tag/cascadiaquakes/blend?label=release&sort=semver)](https://github.com/cascadiaquakes/blend/releases/latest)
[![License](https://img.shields.io/badge/license-LGPL--3.0-blue.svg)](LICENSE)
[![DOI](https://zenodo.org/badge/DOI/10.5281/zenodo.23091422.svg)](https://doi.org/10.5281/zenodo.23091422)

## What is BLEND?

BLEND is a library and collection of command-line programs for localizing functions using arbitrarily shaped support on the regular Cartesian grid. Applications include filter design, spectral analysis, and the synthesis (or merging) of multidimensional models and datasets (e.g., Ajala & Persaud, [2021](https://scholar.google.com/scholar?q=Effect+of+merging+multiscale+models+on+seismic+wavefield+predictions+near+the+southern+San+Andreas+fault), [2022](https://scholar.google.com/scholar?q=Ground-motion+evaluation+of+hybrid+seismic+velocity+models), and Ajala et al. [2025](https://www.researchgate.net/publication/392232576_Toward_an_Accessible_Framework_for_Synthesizing_Solid_Earth_Models_Across_Multiple_Scales)).

Full documentation: [https://ajalalab.com/blend/](https://ajalalab.com/blend/)

## Building and installation

Download the source and build with CMake:

```sh
git clone https://github.com/cascadiaquakes/blend.git
cd blend
cp cmake/ConfigUserTemplate.cmake cmake/ConfigUser.cmake
mkdir build
cd build
cmake ..
cmake --build .
cmake --build . --target install
```

Edit `cmake/ConfigUser.cmake` before configuring if you want to set a custom
installation prefix or enable optional tests, examples, or documentation.

After installation, make sure the installation `bin` directory is in your
`PATH`, then check the following commands:

```sh
blend --version
blend --show-modules
blend --show-windows
```

## Quick examples

Make a 1-D cosine window:

```sh
blend window1d -R0/10 -I1 -Fcosine -T0.2/0.2
```

Query a 1-D cosine window at an arbitrary location:

```sh
printf "2.5\n" | blend window1d -R0/10 -I1 -Fcosine -T0.2/0.2
```

Make a 2-D cosine window using a support/polygon defined in the `polygon.txt` file:

```sh
printf "1 1\n4 1\n4 4\n1 4\n" > polygon.txt
printf "polygon.txt cosine/cosine 0.2/0.2/0.2/0.2\n" > supports.txt
blend window2d -R0/5/0/5 -I0.5 -Bsupports.txt
```

Make a 3-D cosine window using a support/polygon defined in the `polygon.txt` file:

```sh
printf "1 1\n4 1\n4 4\n1 4\n" > polygon.txt
printf "polygon.txt 1 3 cosine/cosine/cosine 0.2/0.2/0.2/0.2/0.2/0.2\n" > supports.txt
blend window3d -R0/5/0/5/0/5 -I0.5 -Bsupports.txt
```

Check if a support/polygon is xy-monotone and make it so if it is not:

```sh
blend monotone polygon.txt -Mb 
```

## Uninstalling

The install step also installs an uninstall helper. Run it from the
installation prefix:

```sh
./share/tools/blend_uninstall.sh
```

Preview removals first with:

```sh
./share/tools/blend_uninstall.sh --dry-run
```

## License

BLEND is licensed under the GNU Lesser General Public License, version 3
or any later version. See `LICENSE` and `NOTICE`
for details.

## Citation

If you use BLEND in your work, please cite the following references:

1. Ajala, R., & Persaud, P. (2021).
   [Effect of merging multiscale models on seismic wavefield predictions near
   the southern San Andreas fault](https://scholar.google.com/scholar?q=Effect+of+merging+multiscale+models+on+seismic+wavefield+predictions+near+the+southern+San+Andreas+fault).
   `Journal of Geophysical Research: Solid Earth`, 126, 1-23.

2. Ajala, R., & Persaud, P. (2022).
   [Ground-motion evaluation of hybrid seismic velocity models](https://scholar.google.com/scholar?q=Ground-motion+evaluation+of+hybrid+seismic+velocity+models).
   `The Seismic Record`, 2, 186-196.

3. Ajala, R., Kolawole, F., Share, P. E., Sahakian, V., Delph, J. R.,
   Hooft, E., & He, B. (2025).
   [Toward an accessible framework for synthesizing solid earth models across
   multiple scales](https://www.researchgate.net/publication/392232576_Toward_an_Accessible_Framework_for_Synthesizing_Solid_Earth_Models_Across_Multiple_Scales).
   In `Seismological Society of America Annual Meeting`, Baltimore, Maryland,
   USA, vol. 96, p. 1364.


## Acknowledgment
The BLEND package design is inspired by the [Generic Mapping Tools (GMT)](https://www.generic-mapping-tools.org/) and was funded by [CRESCENT](https://cascadiaquakes.org) and [NSF](https://www.nsf.gov).
