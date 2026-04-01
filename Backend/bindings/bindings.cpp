#include "bindings.h"

#include <pybind11/pybind11.h>
#include <pybind11/stl.h>

#include "mainModule.h"

namespace py = pybind11;

PYBIND11_MODULE(organ, m) {
    m.doc() = "Organ bindings";

    py::class_<noteSignal>(m, "NoteSignal")
        .def(py::init<unsigned char, unsigned char, bool>(),
            py::arg("note"),
            py::arg("channel"),
            py::arg("on"))

        .def_readwrite("note", &noteSignal::note)
        .def_readwrite("channel", &noteSignal::channel)
        .def_readwrite("on", &noteSignal::on)

        .def("__eq__", &noteSignal::operator==)
        .def("__str__", &noteSignal::toString)
        .def("__repr__", [](const noteSignal& n) { return "<NoteSignal " + n.toString() + ">"; });

    py::class_<mainModule>(m, "MainModule")
        .def(py::init<>())

        .def("play", &mainModule::play)
        .def("get_signal", &mainModule::getSignal)

        .def("get_samples_active", &mainModule::getSamplesActive)
        .def("get_synth_active", &mainModule::getSynthActive)
        .def("get_model_active", &mainModule::getModelActive)

        .def("set_samples_active", &mainModule::setSamplesActive)
        .def("set_synth_active", &mainModule::setSynthActive)
        .def("set_model_active", &mainModule::setModelActive);
}