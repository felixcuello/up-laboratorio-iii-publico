FROM gcc:15.2.0-trixie

# Instalar las dependencias necesarias para el compilador y herramientas de desarrollo
RUN apt update && \
    apt-get install -y \
      cmake \
      curl \
      gdb \
      git \
      htop \
      less \
      make \
      man \
      manpages \
      manpages-dev \
      psmisc \
      unzip \
      vim \
      wget

RUN echo 'syntax on\nset number\nset nocindent\ncolorscheme retrobox\nset tabstop=2\nset shiftwidth=2\nset expandtab\nset autoindent\nset hlsearch\nset incsearch\nset showmatch' > /root/.vimrc

RUN echo "alias l='ls -al'" >> /root/.bashrc

# Instala Neovim 0.11+ (desde el repositorio oficial de Neovim, no desde los repositorios de Debian, porque los
# repositorios de Debian tienen una versión muy antigua)
RUN ARCH="$(dpkg --print-architecture)" && \
    case "$ARCH" in \
      amd64) NVIM_ARCH=x86_64 ;; \
      arm64) NVIM_ARCH=arm64 ;; \
      *) echo "unsupported architecture: $ARCH" && exit 1 ;; \
    esac && \
    curl -fL -o /tmp/nvim.tar.gz "https://github.com/neovim/neovim/releases/download/stable/nvim-linux-${NVIM_ARCH}.tar.gz" && \
    tar -C /opt -xzf /tmp/nvim.tar.gz && \
    mv "/opt/nvim-linux-${NVIM_ARCH}" /opt/nvim && \
    ln -sf /opt/nvim/bin/nvim /usr/local/bin/nvim && \
    rm /tmp/nvim.tar.gz

# tree-sitter CLI (lo usa nvim-treesitter para hacer parseo de código)
# Fijado a 0.24.x: la config usa la rama legacy (master) de nvim-treesitter, que
# pasa el flag '--no-bindings' que el CLI de tree-sitter eliminó en 0.25.0.
RUN ARCH="$(dpkg --print-architecture)" && \
    case "$ARCH" in \
      amd64) TS_ARCH=x64 ;; \
      arm64) TS_ARCH=arm64 ;; \
      *) echo "unsupported architecture: $ARCH" && exit 1 ;; \
    esac && \
    curl -fL -o /tmp/tree-sitter.gz "https://github.com/tree-sitter/tree-sitter/releases/download/v0.24.7/tree-sitter-linux-${TS_ARCH}.gz" && \
    gunzip -c /tmp/tree-sitter.gz > /usr/local/bin/tree-sitter && \
    chmod +x /usr/local/bin/tree-sitter && \
    rm /tmp/tree-sitter.gz

# Node.js: lo usa Copilot (requiere licencia) y tambien tree-sitter para generar
# algunos parsers (p. ej. latex, que se genera desde grammar.js con node).
# Debe instalarse ANTES de generar los parsers de nvim-treesitter.
RUN curl -fsSL https://deb.nodesource.com/setup_22.x | bash - && \
    apt-get install -y nodejs

RUN git clone --depth 1 --filter=blob:none --sparse https://github.com/felixcuello/dotfiles.git /tmp/dotfiles && \
    cd /tmp/dotfiles && \
    git sparse-checkout set nvim && \
    mkdir -p /root/.config && \
    cp -r /tmp/dotfiles/nvim /root/.config/nvim && \
    cd /root && \
    rm -rf /tmp/dotfiles && \
    nvim --headless "+Lazy! restore" +qa && \
    nvim --headless "+TSInstallSync! latex" +qa

WORKDIR /app
