import { useState } from "react";
import Header from "./Components/Header";
import MedicineCard from "./Components/MedicineCard";
import Button from "./Components/Button";

function App() {
  const [medicines, setMedicines] = useState([
    {
      id: 1,
      name: "Paracetamol",
      color: "blue",
      time: "08:00",
    },
    {
      id: 2,
      name: "Ibuprofeno",
      color: "red",
      time: "12:00",
    },
    {
      id: 3,
      name: "Amoxicilina",
      color: "green",
      time: "07:30",
    },
    {
      id: 4,
      name: "Omeprazol",
      color: "purple",
      time: "06:45",
    },
    {
      id: 5,
      name: "Losartana",
      color: "orange",
      time: "20:00",
    },
    {
      id: 6,
      name: "Metformina",
      color: "pink",
      time: "18:00",
    },
    {
      id: 7,
      name: "AAS",
      color: "yellow",
      time: "22:00",
    },
    {
      id: 8,
      name: "Simvastatina",
      color: "teal",
      time: "21:00",
    },
  ]);
  return (
    <main>
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
                  <MedicineCard medicine={medicine} setUpdateMedicine={()=>{}} deleteMedicine={()=>{}} />
                </div>
              ))}
            </div>
          </div>
          <div className="col-span-6  ">
            <div className=" bg-white-main rounded-2xl p-7">
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
              <div>
                <Button text={"Agendar medicamento"}></Button>
              </div>
              <div>
                <Button text={"Notificar atrasos"}></Button>
              </div>
            </div>
          </div>
        </section>
      </section>
    </main>
  );
}

export default App;
