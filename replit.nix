{ pkgs }: {
	deps = [
		pkgs.htop
  pkgs.vim
  	pkgs.pkgconfig
  	pkgs.clang_12
		pkgs.ccls
		pkgs.gdb
		pkgs.gnumake
		pkgs.glfw
		pkgs.libGLU
		pkgs.stb
	];
}