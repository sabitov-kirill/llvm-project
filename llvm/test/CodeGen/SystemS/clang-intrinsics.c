// RUN: %clang -Xclang -triple=systems -S -emit-llvm %s -o - | FileCheck %s --check-prefix=CHECK-IR
// RUN: %clang -Xclang -triple=systems -S %s -o - | FileCheck %s --check-prefix=CHECK-ASM

#include <stdint.h>

typedef int32_t sim_fp;

void test_graphics() {
  // CHECK-IR: call void @llvm.systems.putpixel
  // CHECK-ASM: putpixel
  systemSPutPixel(100, 200, 0xFF);
  
  // CHECK-IR: call i32 @llvm.systems.flush()
  // CHECK-ASM: flush
  systemSFlush();
}

void test_math() {
  // CHECK-IR: call i32 @llvm.systems.fpsqrt
  systemSFPSqrt(1);
  
  // CHECK-IR: call i32 @llvm.systems.fpsin
  systemSFPSin(2);
  
  // CHECK-IR: call i32 @llvm.systems.fpcos
  systemSFPCos(1);
  
  // CHECK-IR: call i32 @llvm.systems.fpatan2
  systemSFPAtan2(1, 1);
}

void test_utilities() {
  // CHECK-IR: call i32 @llvm.systems.deltatime()
  // CHECK-ASM: deltatime
  systemSLog((char *)0x1);
  systemSDeltaTime();
}
