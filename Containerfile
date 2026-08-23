# llvm4msvc 0.9.3, pinned for reproducible local and CI builds.
FROM docker.io/highcanfly/llvm4msvc@sha256:be36de9c334ab15e574ded44935c1e8330d6e16f7bf040abf04eedeeb04b30cd

RUN apt-get update \
    && DEBIAN_FRONTEND=noninteractive apt-get install -y --no-install-recommends \
        ca-certificates \
        curl \
        git \
        ninja-build \
        pkg-config \
        python3 \
        tar \
        unzip \
        zip \
    && rm -rf /var/lib/apt/lists/*

RUN ln -s Version.Lib /usr/share/msvc/sdk/lib/um/x86_64/Version.lib

ARG VCPKG_COMMIT=cc288af760054fa489574bd8e22d05aa8fa01e5c
RUN git clone https://github.com/microsoft/vcpkg.git /opt/vcpkg \
    && git -C /opt/vcpkg checkout "${VCPKG_COMMIT}" \
    && /opt/vcpkg/bootstrap-vcpkg.sh -disableMetrics

ENV VCPKG_ROOT=/opt/vcpkg
ENV VCPKG_DISABLE_METRICS=1
ENV VCPKG_DEFAULT_BINARY_CACHE=/vcpkg-cache
ENV CL=""
ENV LINK=""

WORKDIR /work

ENTRYPOINT ["/work/scripts/build.sh"]
