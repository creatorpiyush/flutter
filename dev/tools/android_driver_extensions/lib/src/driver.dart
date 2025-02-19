// Copyright 2014 The Flutter Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

part of '../native_driver.dart';

/// Drives a native device or emulator that is running a Flutter application.
///
/// Unlike [FlutterDriver], a [NativeDriver] is backed by a platform specific
/// implementation that might interact with out-of-process services, such as
/// `adb` for Android and might require additional setup (e.g., adding test-only
/// plugins to the application under test) for full functionality.
///
/// API that is available directly on [NativeDriver] is considered _lowest
/// common denominator_ and is guaranteed to work on all platforms supported by
/// Flutter Driver unless otherwise noted. Platform-specific functionality that
/// _cannot_ be exposed through this interface is available through
/// platform-specific extensions.
abstract interface class NativeDriver {
  /// Closes the native driver and releases any resources associated with it.
  ///
  /// After calling this method, the driver is no longer usable.
  Future<void> close();

  /// Configures the device for (more) deterministic screenshots if possible.
  ///
  /// This method should be called during `setUpAll` if tests are going to
  /// compare screenshots to golden files.
  ///
  /// Not all platforms support this method, and it is a no-op on platforms that
  /// do not.
  Future<void> configureForScreenshotTesting();

  /// Pings the device to ensure it is responsive.
  ///
  /// The implementation should return a future that completes when a command
  /// is passed to the device (from the driver script), and to the native driver
  /// plugin (if applicable), and back to the driver script.
  ///
  /// The duration of the round trip is returned, typically calculated as:
  /// ```dart
  /// final Stopwatch stopwatch = Stopwatch()..start();
  /// await doPing();
  /// return stopwatch.elapsed;
  /// ```
  Future<Duration> ping();

  /// The SDK version.
  Future<int> get sdkVersion;

  /// Whether the device is an emulator.
  Future<bool> get isEmulator;

  /// Take a screenshot using a platform-specific mechanism.
  ///
  /// The image is returned as an opaque handle that can be used to retrieve
  /// the screenshot data or to compare it with another screenshot, and may
  /// include platform-specific system UI elements, such as the status bar or
  /// navigation bar.
  Future<NativeScreenshot> screenshot();

  /// Rotates the device to landscape orientation.
  Future<void> rotateToLandscape();

  /// Returns the device to its default orientation.
  Future<void> rotateResetDefault();

  /// Taps on a native view found by a selector.
  ///
  /// The [finder] is a platform-specific object that describes how to search
  /// for a view to tap on. The method completes when the tap is complete,
  /// or throws if a view cannot be found or tapped.
  Future<void> tap(NativeFinder finder);
}

/// An opaque handle to a screenshot taken on a native device.
///
/// Unlike [FlutterDriver.screenshot], the screenshot represented by this handle
/// is generated by a platform-specific mechanism and is often already stored
/// on disk. The handle can be used to retrieve the screenshot data or to
/// compare it with another screenshot.
abstract interface class NativeScreenshot {
  /// Saves the screenshot to a file at the specified [path].
  ///
  /// If [path] is not provided, a temporary file will be created.
  ///
  /// Returns the path to the saved file.
  Future<String> saveAs([String? path]);

  /// Reads the screenshot as a PNG-formatted list of bytes.
  Future<Uint8List> readAsBytes();
}
