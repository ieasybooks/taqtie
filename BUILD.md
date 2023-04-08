<h1 dir="rtl">بناء برنامج تقطيع</h1>

<p dir="rtl">ملاحظة: هذه الأوامر تعمل على حاسبي الشخصي، لاستخدامها على أي حاسب آخر يجب تغيير مسارات الملفات حسب الحاجة</p>

<h2 dir="rtl">بناء تقطيع على نظام تشغيل Windows</h2>

<ul dir="rtl">
  <li>قم بتثبيت <code>Qt Creator</code> إصدار <code>6.4.2</code></li>
  <li>قم بتنزيل مستودع تقطيع من خلال تنفيذ الأمر التالي: <code dir="ltr">git clone https://github.com/ieasybooks/taqtie.git</code></li>
  <li>قم بفتح المستودع من خلال <code>Qt Creator</code></li>
  <li>قم بتشغيل برنامج تقطيع داخل <code>Qt Creator</code> باستخدام خيار <code>Release</code></li>
  <li>سيتم إنشاء مجلد جديد باسم <code dir="ltr">build-taqtie-Desktop_Qt_6_4_2_MinGW_64_bit-Release</code>، قم بفتح هذا المجلد في سطر الأوامر (<code>Terminal</code>)</li>
  <li>قم بحذف محتويات المجلد الجديد، إلا ملف <code dir="ltr">تقطيع.exe</code></li>
  <li>قم بتنفيذ الأمر التالي داخل المجلد الجديد: <code dir="ltr">C:\Qt\6.4.2\mingw_64\bin\windeployqt.exe .\تقطيع.exe</code></li>
  <li>بهذا الشكل ستحصل على كل الملفات التي تحتاجها لتشغيل تقطيع على أي حاسب آخر، يمكنك ضغط المجلد ونقله الآن</li>
</ul>

<h2 dir="rtl">بناء تقطيع على نظام تشغيل MacOS</h2>

<ul dir="rtl">
  <li>قم بتثبيت <code>Qt Creator</code> إصدار <code>6.4.2</code></li>
  <li>قم بتنزيل مستودع تقطيع من خلال تنفيذ الأمر التالي: <code dir="ltr">git clone https://github.com/ieasybooks/taqtie.git</code></li>
  <li>قم بفتح المستودع من خلال <code>Qt Creator</code></li>
  <li>قم بتشغيل برنامج تقطيع داخل <code>Qt Creator</code> باستخدام خيار <code>Release</code></li>
  <li>سيتم إنشاء مجلد جديد باسم <code dir="ltr">build-repo-Qt_6_4_0_for_macOS-Release</code>، قم بفتح هذا المجلد في سطر الأوامر (<code>Terminal</code>)</li>
  <li>قم بتنفيذ الأوامر التالية داخل المجلد الجديد:</li>
</ul>

```
install_name_tool -delete_rpath /Users/aliosm/Qt/6.4.0/macos/lib تقطيع.app/Contents/MacOS/تقطيع
install_name_tool -add_rpath @executable_path/../Frameworks تقطيع.app/Contents/MacOS/تقطيع
/Users/aliosm/Qt/6.4.0/macos/bin/macdeployqt تقطيع.app -dmg -appstore-compliant -always-overwrite
```

<ul dir="rtl">
  <li>بهذا الشكل ستحصل على ملف <code dir="ltr">تقطيع.dmg</code> والذي يمكن تشغيله على أي حاسب آخر</li>
</ul>
