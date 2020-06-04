#!/usr/bin/env bash

OS="`uname -s`"

if [[ "$OS" == "Linux" ]] ; then
  container_name=emscripten_damerau_levenshtein

  stop() {
    docker kill $container_name 1>/dev/null 2>&1
    docker rm $container_name 1>/dev/null 2>&1
  }

  die() {
    echo "$*" 1>&2
    stop
    exit 1
  }

  echo "Build Node binaries"
  yarn build:node || die "Failed to build Node binary"

  echo "Build WASM"
  mkdir -p $(pwd)/dist
  docker run -dit --name $container_name -v $(pwd):/src trzeci/emscripten-slim:sdk-tag-1.39.4-64bit bash \
    || die "Failed to initialize Emscripten Docker image"
  docker exec -it $container_name npm run build:wasm \
    || die "Failed to build WASM"
  stop

elif [[ "$OS" == "Darwin" ]] ; then
  echo 'TODO'
fi