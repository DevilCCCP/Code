msvc: LIB_EXT=lib
gcc:  LIB_EXT=a

msvc: LIB_PREFIX=
gcc:  LIB_PREFIX=lib


for(dep, DEPEND_LIBS) {
  #message($$TARGET depends on $$dep ($${INNER_LIB_DIR}/$${LIB_PREFIX}$${dep}.$${LIB_EXT}))
  LIBS += -l$${dep}
  PRE_TARGETDEPS += $${INNER_LIB_DIR}/$${LIB_PREFIX}$${dep}.$${LIB_EXT}
}

