bl_info = {
    "name": "QUE Exporter",
    "author": "kubs",
    "version": (1, 0),
    "blender": (4, 0, 0),
    "location": "View3D > Sidebar > QUE Export",
    "description": "Exports levels and models for QUE engine",
    "warning": "",
    "category": "Import-Export"
}

from . import io_que_export_level
from . import io_que_export_model

def register():
    io_que_export_level.register()
    io_que_export_model.register()

def unregister():
    io_que_export_level.unregister()
    io_que_export_model.unregister()

if __name__ == "__main__":
    register()