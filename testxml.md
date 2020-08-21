---
layout: default
---

# test xml

```info
this is a test with a multiline note that should have a red bar just in front. and the linebreak should be totaly automatic.this is a test with a multiline note that should have a red bar just in front. and the linebreak should be totaly automaticthis is a test with a multiline note that should have a red bar just in front. and the linebreak should be totaly automaticthis is a test with a multiline note that should have a red bar just in front. and the linebreak should be totaly automaticthis is a test with a multiline note that should have a red bar just in front. and the linebreak should be totaly automatic
```
```warning
this is a test with a multiline note that should have a red bar just
in front. and the linebreak should be totaly automatic
```
```danger
this is a test with a multiline note that should have a red bar just
in front. and the linebreak should be totaly automatic
```
```csharp
if (createSurface)
    initSurface ();

if (startUIThread) {
    Thread t = new Thread (InterfaceThread) {
        IsBackground = true
    };
    t.Start ();
}
```

{% hello Jesse %}

## Structures
<ul>
{% assign structs = site.data.doc.xml.index.doxygenindex.compound | where: "kind", "struct" %}
{% for struct in structs %}
    <li>
        {{struct.name}}
        <ul>
        {%for mb in struct.member%}
            <li>{{ mb.name }}</li>
        {% endfor %}
        </ul>
    </li>
{% endfor %}
</ul>

## Files
<ul>
{% assign files = site.data.doc.xml.index.doxygenindex.compound | where: "kind", "file" %}
{% for file in files %}
    <li>
        {{file.name}}
        <ul>
        {%for mb in file.member%}
            <li>{{ mb.name }}</li>
        {% endfor %}
        </ul>
    </li>
{% endfor %}
</ul>