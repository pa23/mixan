# Maintainer: Artem Petrov <pa2311@gmail.com>

pkgname=mixan
pkgver=1.8.3
pkgrel=1
epoch=
pkgdesc="Analyze of granular material mix and emulsions."
arch=('x86_64')
url="https://github.com/pa23/mixan"
license=('GPL3')
groups=()
depends=('qt' 'qwt')
makedepends=('qt' 'qwt')
checkdepends=()
optdepends=()
provides=()
conflicts=()
replaces=()
backup=()
options=()
install=
changelog=
source=($pkgname-$pkgver-src.tar.gz)
noextract=()
md5sums=('') #generate with 'makepkg -g'

build() {
    cd "$srcdir/$pkgname-$pkgver"
    qmake PREFIX=/usr mixan.pro
    make
}

package() {
    cd ${srcdir}/$pkgname-$pkgver
    make INSTALL_ROOT="$pkgdir" install
}

