#ifndef __STABLE_HPP__
#include <core/stable.hpp>
#endif

#include <gui/widgets/pagenym_source.hpp>
#include <ui_pagenym_source.h>

#include <namecoin/Namecoin.hpp>

#include <string>

MTPageNym_Source::MTPageNym_Source(QWidget *parent) :
    QWizardPage(parent),
    ui(new Ui::MTPageNym_Source)
{
    ui->setupUi(this);

    this->registerField("Source*", ui->lineEditSource);
}

//virtual
void MTPageNym_Source::showEvent(QShowEvent * event)
{
    // -------------------------------
    // call inherited method
    //
    QWizardPage::showEvent(event);
    // -------------------------------
    int nAuthorityIndex = field("Authority").toInt();

    // Default is no extra label and no button.
    ui->labelExtra->setText("");
    ui->getAddress->setVisible(false);

    if (1 == nAuthorityIndex) // Namecoin
    {
        ui->label->setText(tr("Namecoin Address:"));
        ui->labelExplanation->setText(tr("Enter the Namecoin address that will be authoritative over this identity."));

        NMC_Interface nmc;
        std::string msg;
        nmcrpc::CoinInterface& nc = nmc.getNamecoin ();
        const bool ok = nc.testConnection (msg);

        if (ok)
        {
            const nmcrpc::CoinInterface::Balance b = nc.getBalance ();
            msg += " Your balance: " + b.toString () + " NMC.";

            ui->getAddress->setVisible(true);
            ui->getAddress->setText(tr("Get Address"));

            connect(ui->getAddress, SIGNAL(clicked()),
                    this, SLOT(getAddressClicked()));
        }

        ui->labelExtra->setText(msg.c_str ());
    }
    else if (2 == nAuthorityIndex) // Legacy CA
    {
        ui->label->setText(tr("DN info:"));
        ui->labelExplanation->setText(tr("Enter the DN info that uniquely identifies you on your x.509 certificate."));
    }
}

//virtual
bool MTPageNym_Source::validatePage()
{
  const int nAuthorityIndex = field("Authority").toInt();

  /* In the case of Namecoin source, check that the Namecoin address
     entered is valid and owned by the user.  */
  if (nAuthorityIndex == 1)
    {
      const std::string addrStr = ui->lineEditSource->text().toStdString();

      NMC_Interface nmc;
      nmcrpc::CoinInterface& nc = nmc.getNamecoin();
      const bool ok = nc.testConnection ();

      if (!ok)
        {
          // The connection error message should still be shown.
          return false;
        }

      const nmcrpc::CoinInterface::Address addr = nc.queryAddress(addrStr);
      if (!addr.isValid ())
        {
          ui->labelExtra->setText(tr("Please enter a valid Namecoin address."));
          return false;
        }
      if (!addr.isMine ())
        {
          ui->labelExtra->setText(tr("The provided Namecoin address is not in your wallet."));
          return false;
        }

      /* Test the wallet balance.  We want at least 0.05 NMC to have some
         safety margin over the minimal cost of 0.02 NMC.  */
      const nmcrpc::CoinInterface::Balance b = nc.getBalance ();
      if (20 * b.getIntValue () < nmcrpc::CoinInterface::Balance::COIN)
        {
          ui->labelExtra->setText(tr("Insufficient balance! You need at least 0.05 NMC in your wallet."));
          return false;
        }
    }

  return true;
}

MTPageNym_Source::~MTPageNym_Source()
{
    delete ui;
}

void MTPageNym_Source::getAddressClicked()
{
  NMC_Interface nmc;
  nmcrpc::CoinInterface::Address addr;
  addr = nmc.getNamecoin().createAddress();

  ui->lineEditSource->setText(addr.getAddress().c_str());
}
