ملاحظة: هذه الأوامر تعمل على حاسبي الشخصي، لاستخدامها على أي حاسب آخر يجب تغيير مسارات الملفات حسب الحاجة

# بناء تقطيع على نظام تشغيل Windows

```
C:\Qt\6.4.2\mingw_64\bin\windeployqt.exe .\تقطيع.exe
```

# بناء تقطيع على نظام تشغيل MacOS

```
install_name_tool -delete_rpath /Users/aliosm/Qt/6.4.0/macos/lib تقطيع.app/Contents/MacOS/تقطيع
install_name_tool -add_rpath @executable_path/../Frameworks تقطيع.app/Contents/MacOS/تقطيع
/Users/aliosm/Qt/6.4.0/macos/bin/macdeployqt تقطيع.app -dmg -appstore-compliant -always-overwrite
```
