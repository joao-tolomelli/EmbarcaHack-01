import logo from "../../assets/logo.svg";

function Header() {
  return (
    <div className="bg-green-main rounded-b-xl px-[4rem] py-[1rem] flex flex-row justify-between">
      <div className="flex flex-row justify-center items-center gap-4">
        <img className="w-[2.5rem]" src={logo} alt="Logo" />
        <h1 className="text-3xl text-white-main font-bold">Re-Mem</h1>
      </div>
        <div className="flex flex-col justify-center">
          <i className="pi pi-sign-out text-white text-2xl"></i>
        </div>
    </div>
  );
}

export default Header;
