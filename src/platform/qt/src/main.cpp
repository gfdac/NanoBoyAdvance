/*
 * Copyright (C) 2021 fleroviux
 *
 * Licensed under GPLv3 or any later version.
 * Refer to the included LICENSE file.
 */

#include <filesystem>
#include <QApplication>
#include <QSurfaceFormat>
#include <stdlib.h>

#include "widget/main_window.hpp"

#if defined(WIN32)
  #include <QtPlatformHeaders/QWindowsWindowFunctions>
#endif

namespace fs = std::filesystem;

void parse_args(MainWindow& window, int argc, char** argv) {
  if (argc != 2) {
    return;
  }

  auto rom = fs::path{argv[1]};

  if (rom.is_relative()) {
    rom = fs::current_path() / rom;
  }
  fs::current_path(fs::path{argv[0]}.remove_filename());

  window.LoadROM(rom.string());
}

int main(int argc, char** argv) {
  // See: https://trac.wxwidgets.org/ticket/19023
#if defined(__APPLE__)
  setenv("LC_NUMERIC", "C", 1);
#endif

  // On some systems (e.g. macOS) QSurfaceFormat::setDefaultFormat() must be called before constructing QApplication.
  auto format = QSurfaceFormat{};
  format.setProfile(QSurfaceFormat::CoreProfile);
  format.setMajorVersion(3);
  format.setMinorVersion(3);
  format.setSwapInterval(0);
  QSurfaceFormat::setDefaultFormat(format);

  QApplication app{ argc, argv };

#if defined(WIN32)
  // See: https://doc.qt.io/qt-5/windows-issues.html#fullscreen-opengl-based-windows
  QWindowsWindowFunctions::setHasBorderInFullScreenDefault(true);
#endif

  MainWindow window{ &app };

  QCoreApplication::setOrganizationName("fleroviux");
  QCoreApplication::setApplicationName("NanoBoyAdvance");

  window.show();

  parse_args(window, argc, argv);

  return app.exec();
}
