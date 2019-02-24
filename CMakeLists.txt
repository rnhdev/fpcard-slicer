cmake_minimum_required(VERSION 2.8)
project(fpcard_slicer)

set(CMAKE_CXX_STANDARD 11)
set(NBIS_PATH ${CMAKE_CURRENT_SOURCE_DIR}/third_party/nbis)

SET(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-unused-parameter -std=c++11 -Wall  -Wno-reorder -Wno-deprecated-declarations")

link_directories(${NBIS_PATH}/exports/lib)

set(INCLUDE_PATH
    include
    ${NBIS_PATH}/exports/include)

set(NBIS_OBJ_LIST
    ${NBIS_PATH}/imgtools/obj/src/lib/ihead/getnset.o
    ${NBIS_PATH}/imgtools/obj/src/lib/ihead/getcomp.o
    ${NBIS_PATH}/imgtools/obj/src/lib/ihead/nullihdr.o
    ${NBIS_PATH}/imgtools/obj/src/lib/ihead/parsihdr.o
    ${NBIS_PATH}/imgtools/obj/src/lib/ihead/prntihdr.o
    ${NBIS_PATH}/imgtools/obj/src/lib/ihead/readihdr.o
    ${NBIS_PATH}/imgtools/obj/src/lib/ihead/valdcomp.o
    ${NBIS_PATH}/imgtools/obj/src/lib/ihead/writihdr.o

    ${NBIS_PATH}/imgtools/obj/src/lib/wsq/util.o
    ${NBIS_PATH}/imgtools/obj/src/lib/wsq/cropcoeff.o
    ${NBIS_PATH}/imgtools/obj/src/lib/wsq/decoder.o
    ${NBIS_PATH}/imgtools/obj/src/lib/wsq/encoder.o
    ${NBIS_PATH}/imgtools/obj/src/lib/wsq/globals.o
    ${NBIS_PATH}/imgtools/obj/src/lib/wsq/huff.o
    ${NBIS_PATH}/imgtools/obj/src/lib/wsq/ppi.o
    ${NBIS_PATH}/imgtools/obj/src/lib/wsq/tableio.o
    ${NBIS_PATH}/imgtools/obj/src/lib/wsq/tree.o
    ${NBIS_PATH}/imgtools/obj/src/lib/wsq/sd14util.o

    ${NBIS_PATH}/imgtools/obj/src/lib/jpegl/decoder.o
    ${NBIS_PATH}/imgtools/obj/src/lib/jpegl/encoder.o
    ${NBIS_PATH}/imgtools/obj/src/lib/jpegl/huff.o
    ${NBIS_PATH}/imgtools/obj/src/lib/jpegl/huftable.o
    ${NBIS_PATH}/imgtools/obj/src/lib/jpegl/imgdat.o
    ${NBIS_PATH}/imgtools/obj/src/lib/jpegl/ppi.o
    ${NBIS_PATH}/imgtools/obj/src/lib/jpegl/sd4util.o
    ${NBIS_PATH}/imgtools/obj/src/lib/jpegl/tableio.o
    ${NBIS_PATH}/imgtools/obj/src/lib/jpegl/util.o

    ${NBIS_PATH}/imgtools/obj/src/lib/image/bincopy.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/binfill.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/binpad.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/bitmasks.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/dilate.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/findblob.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/grp4comp.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/grp4deco.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/imageops.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/img_io.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/imgavg.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/imgboost.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/imgdecod.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/imgutil.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/imgsnip.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/imgtype.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/intrlv.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/masks.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/parsargs.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/readihdr.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/rgb_ycc.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/rl.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/sunrast.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/thresh.o
    ${NBIS_PATH}/imgtools/obj/src/lib/image/writihdr.o
    )

add_executable(fpcard_slicer
    ${NBIS_OBJ_LIST} include/image.h src/image.cpp src/slicer.cpp include/slicer.h src/main.cpp)
target_include_directories(fpcard_slicer PRIVATE ${INCLUDE_PATH})

target_link_libraries(fpcard_slicer
    m
    liban2k.a
    libbozorth3.a
    libcblas.a
    libclapck.a
    libf2c.a
    libfet.a
    libfft.a
    libihead.a
    libimage.a
    libioutil.a
    libjpegb.a
    libjpegl.a
    libmindtct.a
    libmlp.a
    libnfiq.a
    libnfseg.a
    libopenjp2.a
    libpca.a
    libpcautil.a
    libpng.a
    libutil.a
    libwsq.a
    libz.a
    )


