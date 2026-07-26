FROM gcc:15.2.0-trixie

RUN 

# Install common development tools
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
      neovim \
      psmisc \
      unzip \
      vim \
      wget

RUN echo 'syntax on\nset number\nset nocindent\ncolorscheme retrobox\nset tabstop=2\nset shiftwidth=2\nset expandtab\nset autoindent\nset hlsearch\nset incsearch\nset showmatch' > /root/.vimrc

WORKDIR /app
