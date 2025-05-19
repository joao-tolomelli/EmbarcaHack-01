function MedicineCard({ medicine, setUpdateMedicine, deleteMedicine }) {
  const renderIcon = (status) => {
    switch (status) {
      case "ingerido":
        return <i className="pi pi-check-circle text-green-700  text-base"></i>;
      case "ignorado":
        return (
          <i className="pi pi-exclamation-triangle text-red-700 text-base"></i>
        );
      case "pendente":
        return <i className="pi pi-clock text-gray-700 text-base"></i>;
      default:
        return null;
    }
  };

  return (
    <div
      className={`flex flex-row justify-between items-center bg-green-main text-white text-[.7rem] rounded-xl px-6 py-2 ${
        medicine.status === "ignorado" && !(setUpdateMedicine && deleteMedicine)
          ? "border-2 border-red-700"
          : ""
      }`}
    >
      <div className="flex flex-col font-normal gap-1">
        <span>Nome: {medicine.name}</span>
        <span>Cor: {medicine.color}</span>
        <span>Horário: {medicine.time}</span>
      </div>
      {setUpdateMedicine && deleteMedicine ? (
        <div className="flex flex-col gap-6">
          <div onClick={() => setUpdateMedicine(medicine.id)}>
            <i className="pi pi-file-edit text-white text-base cursor-pointer"></i>
          </div>
          <div onClick={() => deleteMedicine(medicine.id)}>
            <i className="pi pi-trash text-white text-base cursor-pointer"></i>
          </div>
        </div>
      ) : (
        <div>{renderIcon(medicine.status)}</div>
      )}
    </div>
  );
}

export default MedicineCard;
