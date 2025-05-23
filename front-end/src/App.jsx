import { useEffect, useState } from "react";
import Header from "./Components/Header";
import MedicineCard from "./Components/MedicineCard";
import Button from "./Components/Button";
import InputBox from "./Components/InputBox";
import axios from "axios";

function App() {
  const BASE_URL = import.meta.env.VITE_API_BASE_URL;

  const [medicines, setMedicines] = useState(null);
  const [showForm, setShowForm] = useState(false);
  const [showDeletePopup, setShowDeletePopup] = useState(false);
  const [pendingDeleteId, setPendingDeleteId] = useState(null);

  const [formData, setFormData] = useState({
    name: "",
    color: "",
    time: "",
    status: "pendente",
  });

  const [formDataSettings, setFormDataSettings] = useState({
    action: "", // 'create' or 'update'
    buttonText: "",
    header: "",
    id: null,
  });

  useEffect(() => {
    fetchHomeData();
  }, []);

  const fetchHomeData = async () => {
    try {
      const response = await axios.get(`${BASE_URL}/api/medicines`);
      setMedicines(response.data);
    } catch (error) {
      console.error("Erro ao buscar dados da home:", error);
    }
  };

  const scheduleMedicines = async () => {
    try {
      await axios.post(`${BASE_URL}/api/medicines`, formData);
      console.log("Medicamento registrado com sucesso!");
      await fetchHomeData();
    } catch (error) {
      console.error("Erro ao registrar medicamento:", error);
    }
  };

  const updateMedicines = async (id) => {
    try {
      const formToSend = { ...formData, id };
      await axios.put(`${BASE_URL}/api/medicines/${id}`, formToSend);
      console.log("Medicamento atualizado com sucesso!");
      await fetchHomeData();
    } catch (error) {
      console.error("Erro ao atualizar medicamento:", error);
    }
  };

  const deleteMedicine = (id) => {
    setPendingDeleteId(id);
    setShowDeletePopup(true);
  };

  const confirmDelete = async () => {
    try {
      await axios.delete(`${BASE_URL}/api/medicines/${pendingDeleteId}`);
      await fetchHomeData();
      console.log("Medicamento deletado com sucesso!");
    } catch (error) {
      console.error("Erro ao deletar medicamento:", error);
    } finally {
      setShowDeletePopup(false);
      setPendingDeleteId(null);
    }
  };

  const setUpdateService = (id) => {
    setFormDataSettings({
      action: "update",
      buttonText: "Atualizar",
      header: "Atualizar medicamento",
      id: id,
    });
    const medicineToUpdate = medicines.find((medicine) => medicine.id === id);
    setFormData(medicineToUpdate);
    setShowForm(true);
  };

  const handleScheduleClick = () => {
    setFormDataSettings({
      action: "create",
      buttonText: "Agendar",
      header: "Agendar novo medicamento",
      id: null,
    });
    setFormData({ name: "", color: "", time: "", status: "pendente" });
    setShowForm(true);
  };

  const handleFormButton = () => {
    if (formDataSettings.action === "create") {
      scheduleMedicines();
    } else if (formDataSettings.action === "update") {
      updateMedicines(formDataSettings.id);
    }
    setShowForm(false);
    setFormData({ name: "", color: "", time: "", status: "pendente" });
  };

  const notifyDelay = async () => {
    try {
      const response = await axios.get(`${BASE_URL}/api/medicines/ignorados`);
    } catch (error) {
      console.error("Erro ao buscar dados da home:", error);
    }
  };

  const changeInfo = (key, setter) => (e) => {
    setter((prev) => ({
      ...prev,
      [key]: e.target.value,
    }));
  };

  if (!medicines) {
    return (
      <>
        <Header />
        <div className="h-screen w-screen flex items-center justify-center">
          <p className="text-xl font-semibold">Carregando informações...</p>
        </div>
      </>
    );
  }

  return (
    <main className="relative">
      <Header />
      <section className="px-[4rem] pb-4">
        <h1 className="text-2xl text-default-text font-medium mt-12 mb-6">
          Bem vindo, Gabriel!
        </h1>
        <section className="grid grid-cols-12 gap-4">
          <div className="col-span-6 bg-white-main rounded-2xl p-7">
            <h1 className="text-xl text-default-text font-medium mb-4">
              Medicamentos agendados
            </h1>
            <div className="flex flex-col h-[22rem] overflow-y-scroll gap-3">
              {medicines.map((medicine, index) => (
                <div key={index} className="pr-6">
                  <MedicineCard
                    medicine={medicine}
                    setUpdateMedicine={setUpdateService}
                    deleteMedicine={deleteMedicine}
                  />
                </div>
              ))}
            </div>
          </div>
          <div className="col-span-6">
            <div className="bg-white-main rounded-2xl p-7">
              <h1 className="text-xl text-default-text font-medium mb-4">
                Relátorio diário
              </h1>
              <div className="flex flex-col h-[14rem] overflow-y-scroll gap-3">
                {medicines.map((medicine, index) => (
                  <div key={index} className="pr-6">
                    <MedicineCard medicine={medicine} />
                  </div>
                ))}
              </div>
            </div>

            <div className="flex flex-col px-40 pt-8 gap-6">
              <div onClick={handleScheduleClick}>
                <Button text={"Agendar medicamento"} />
              </div>
              <div onClick={notifyDelay}>
                <Button text={"Notificar atrasos"} />
              </div>
            </div>
          </div>
        </section>
      </section>

      {showForm && (
        <div className="fixed top-0 left-0 w-full h-full bg-black bg-opacity-40 flex justify-center items-center z-50">
          <div className="bg-white p-6 rounded-2xl shadow-xl w-[24rem] relative">
            <div className="flex justify-between items-center mb-4">
              <h2 className="text-lg font-medium text-center flex-1">
                {formDataSettings.header}
              </h2>
              <button
                onClick={() => setShowForm(false)}
                className="text-gray-500 hover:text-red-600 text-xl font-bold ml-4"
                title="Fechar"
              >
                ×
              </button>
            </div>

            <div className="flex flex-col gap-4">
              <InputBox
                label="Nome"
                value={formData.name}
                onChange={changeInfo("name", setFormData)}
              />
              <InputBox
                label="Cor"
                value={formData.color}
                onChange={changeInfo("color", setFormData)}
              />
              <InputBox
                label="Horário"
                type="time"
                value={formData.time}
                onChange={changeInfo("time", setFormData)}
              />
              <div onClick={handleFormButton}>
                <Button text={formDataSettings.buttonText} />
              </div>
            </div>
          </div>
        </div>
      )}

      {showDeletePopup && (
        <div className="fixed top-0 left-0 w-full h-full bg-black bg-opacity-40 flex justify-center items-center z-50">
          <div className="bg-white p-6 rounded-2xl shadow-xl w-[24rem] relative">
            <h2 className="text-lg font-medium mb-4">Confirmar exclusão</h2>
            <p className="mb-6 text-gray-700">
              Tem certeza que deseja excluir este medicamento?
            </p>
            <div className="flex justify-end gap-4">
              <button
                onClick={() => {
                  setShowDeletePopup(false);
                  setPendingDeleteId(null);
                }}
                className="px-4 py-2 rounded bg-gray-300 hover:bg-gray-400"
              >
                Cancelar
              </button>
              <button
                onClick={confirmDelete}
                className="px-4 py-2 rounded bg-red-500 text-white hover:bg-red-600"
              >
                Excluir
              </button>
            </div>
          </div>
        </div>
      )}
    </main>
  );
}

export default App;
